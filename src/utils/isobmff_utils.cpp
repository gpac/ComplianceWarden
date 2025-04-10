#include "core/box.h"
#include "core/fourcc.h"
#include "core/spec.h"

#include <cstring> // strcmp
#include <map>
#include <vector>

bool isIsobmff(Box const &root)
{
  return !root.children.empty();
}

std::vector<const Box *> findBoxes(const Box &root, uint32_t fourcc)
{
  std::vector<const Box *> res;

  if(root.fourcc == fourcc)
    res.push_back(&root);

  for(auto &box : root.children) {
    if(box.fourcc == fourcc) {
      res.push_back(&box);
    } else {
      auto b = findBoxes(box, fourcc);
      res.insert(res.end(), b.begin(), b.end());
    }
  }

  return res;
}

Box const &getBoxFromOffset(Box const &root, uint64_t targetOffset)
{
  for(auto &box : root.children)
    if(box.position + box.size > targetOffset)
      return getBoxFromOffset(box, targetOffset);

  return root;
}

void checkEssential(Box const &root, IReport *out, uint32_t fourcc)
{
  std::vector<uint32_t> properties{ 0 }; // index is 1-based

  for(auto &box : root.children)
    if(box.fourcc == FOURCC("meta"))
      for(auto &metaChild : box.children)
        if(metaChild.fourcc == FOURCC("iprp"))
          for(auto &iprpChild : metaChild.children)
            if(iprpChild.fourcc == FOURCC("ipco"))
              for(auto &ipcoChild : iprpChild.children)
                properties.push_back(ipcoChild.fourcc);

  for(auto &box : root.children)
    if(box.fourcc == FOURCC("meta"))
      for(auto &metaChild : box.children)
        if(metaChild.fourcc == FOURCC("iprp"))
          for(auto &iprpChild : metaChild.children)
            if(iprpChild.fourcc == FOURCC("ipma")) {
              bool essential = false;
              uint32_t itemId = 0;

              for(auto &sym : iprpChild.syms) {
                if(!strcmp(sym.name, "item_ID"))
                  itemId = sym.value;
                else if(!strcmp(sym.name, "essential"))
                  essential = sym.value;
                else if(!strcmp(sym.name, "property_index")) {
                  if(sym.value > (int64_t)properties.size()) {
                    out->error("property_index \"%lld\" doesn't exist (%u detected).", sym.value, properties.size());
                    break;
                  }

                  if(properties[sym.value] == fourcc) {
                    out->covered();
                    if(!essential)
                      out->error(
                        "Property \"%s\" shall be marked as essential (item_ID=%u)",
                        toString(properties[sym.value]).c_str(), itemId);
                    }
                }
              }
            }
}

std::vector<std::pair<int64_t /*offset*/, int64_t /*length*/>>
getItemDataOffsets(Box const &root, IReport *out, uint32_t itemID)
{
  struct ItemLocation {
    int construction_method = 0, data_reference_index = 0;
    int64_t base_offset = 0;
    std::vector<std::pair<int64_t /*offset*/, int64_t /*length*/>> extents;
  };

  std::vector<ItemLocation> itemLocs;

  for(auto &meta : root.children)
    if(meta.fourcc == FOURCC("meta"))
      for(auto &metaChild : meta.children)
        if(metaChild.fourcc == FOURCC("iloc")) {
          int64_t currOffset = 0;

          for(auto &sym : metaChild.syms) {
            if(!strcmp(sym.name, "item_ID")) {
              if(sym.value != itemID) {
                if(itemLocs.empty())
                  continue;
                else
                  break;
              }

              itemLocs.resize(itemLocs.size() + 1);

              continue;
            }

            if(itemLocs.empty())
              continue;

            auto &itemLoc = itemLocs.back();

            if(!strcmp(sym.name, "construction_method")) {
              itemLoc.construction_method = sym.value;

              if(itemLoc.construction_method == 1) {
                auto idat = findBoxes(meta, FOURCC("idat"));

                if(idat.size() == 1)
                  itemLoc.base_offset = idat[0]->position + 8;
                else
                  out->error("construction_method=1 but found %llu \"idat\" boxes instead of 1", idat.size());
              } else if(itemLoc.construction_method > 1)
                out->warning("construction_method > 1 not supported");
            }

            if(!strcmp(sym.name, "data_reference_index")) {
              itemLoc.data_reference_index = sym.value;

              if(itemLoc.data_reference_index > 0)
                out->warning("data_reference_index > 0 not supported");
            }

            if(!strcmp(sym.name, "base_offset"))
              itemLoc.base_offset += sym.value;

            if(!strcmp(sym.name, "extent_offset"))
              currOffset = sym.value;

            if(!strcmp(sym.name, "extent_length"))
              itemLoc.extents.push_back({ currOffset, sym.value });
          }
        }

  std::vector<std::pair<int64_t /*offset*/, int64_t /*length*/>> spans;

  if(itemLocs.empty())
    return spans;

  if(itemLocs.size() > 1)
    out->error("More than one 'iloc' found for Item_ID=%u. Only considering the first one.", itemID);

  auto &itemLoc = itemLocs[0];

  if(itemLoc.extents.empty() && itemLoc.base_offset) {
    // allows to pass our own tests
    spans.push_back({ itemLoc.base_offset, 0 });
  } else {
    for(auto &extent : itemLoc.extents)
      // we assume no idat-based check (construction_method = 1)
      spans.push_back({ itemLoc.base_offset + extent.first, extent.second });
  }

  return spans;
}

void boxCheck(
  Box const &root, IReport *out, std::vector<uint32_t> oneOf4CCs, std::vector<uint32_t> parent4CCs,
  std::pair<unsigned, unsigned> expectedAritySpan)
{
  std::vector<const Box *> parents;

  for(auto parent4CC : parent4CCs) {
    auto b = findBoxes(root, parent4CC);
    parents.insert(parents.end(), b.begin(), b.end());
  }

  for(auto &parent : parents) {
    unsigned arityFromParent = 0;

    for(auto fourcc : oneOf4CCs) {
      unsigned localArity = 0;

      if(parent->fourcc == fourcc) {
        localArity++;
      } else {
        for(auto &child : parent->children)
          if(child.fourcc == fourcc)
            localArity++;
      }

      arityFromParent += localArity;
    }

    if(arityFromParent < expectedAritySpan.first || arityFromParent > expectedAritySpan.second) {
      std::string oneOf4CCsStr, parent4CCsStr;

      for(auto fourcc : oneOf4CCs)
        oneOf4CCsStr += toString(fourcc) + " ";

      for(auto fourcc : parent4CCs)
        parent4CCsStr += toString(fourcc) + " ";

      out->error(
        "Wrong arity for boxes { %s} in parents { %s}: expected in range [%u-%u], found %u", oneOf4CCsStr.c_str(),
        parent4CCsStr.c_str(), expectedAritySpan.first, expectedAritySpan.second, arityFromParent);
    }
  }
}

std::vector<uint32_t /*itemId*/> findImageItems(Box const &root, uint32_t fourcc)
{
  std::vector<uint32_t> imageItemIDs;

  // Find Image Items
  for(auto &box : root.children)
    if(box.fourcc == FOURCC("meta"))
      for(auto &metaChild : box.children)
        if(metaChild.fourcc == FOURCC("iinf"))
          for(auto &iinfChild : metaChild.children)
            if(iinfChild.fourcc == FOURCC("infe")) {
              uint32_t itemId = 0;

              for(auto &sym : iinfChild.syms) {
                if(!strcmp(sym.name, "item_ID"))
                  itemId = sym.value;
                else if(!strcmp(sym.name, "item_type"))
                  if(sym.value == fourcc)
                    imageItemIDs.push_back(itemId);
              }
            }

  return imageItemIDs;
}

std::vector<const Box *> findBoxesWithProperty(Box const &root, uint32_t itemId, uint32_t fourcc)
{
  struct Entry {
    int found = 0;
    const Box *box = nullptr;
  };
  std::map<uint32_t /*property index*/, Entry> propertyIndex;

  for(auto &box : root.children)
    if(box.fourcc == FOURCC("meta"))
      for(auto &metaChild : box.children)
        if(metaChild.fourcc == FOURCC("iprp"))
          for(auto &iprpChild : metaChild.children)
            if(iprpChild.fourcc == FOURCC("ipco"))
              for(uint32_t i = 1; i <= iprpChild.children.size(); ++i)
                if(iprpChild.children[i - 1].fourcc == fourcc)
                  propertyIndex.insert({ i, { 0, &iprpChild.children[i - 1] } });

  for(auto &box : root.children)
    if(box.fourcc == FOURCC("meta"))
      for(auto &metaChild : box.children)
        if(metaChild.fourcc == FOURCC("iprp"))
          for(auto &iprpChild : metaChild.children)
            if(iprpChild.fourcc == FOURCC("ipma")) {
              uint32_t localItemId = 0;

              for(auto &sym : iprpChild.syms) {
                if(!strcmp(sym.name, "item_ID"))
                  localItemId = sym.value;
                else if(!strcmp(sym.name, "property_index"))
                  if(localItemId == itemId)
                    for(auto &a : propertyIndex)
                      if(a.first == sym.value)
                        a.second.found++;
              }
            }

  std::vector<const Box *> propertyBoxes;

  for(auto &a : propertyIndex)
    for(int found = 0; found < a.second.found; ++found)
      propertyBoxes.push_back(a.second.box);

  return propertyBoxes;
}
