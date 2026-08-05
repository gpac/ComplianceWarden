#include "gimi_utils.h"

#include <stdexcept>

std::vector<const Box *> get_all_metaboxes(const Box &root)
{
  std::vector<const Box *> metaboxes;

  // File Level Metaboxes
  for(const Box &box : root.children) {
    if(box.fourcc == FOURCC("meta")) {
      metaboxes.push_back(&box);
    }
  }

  // Track Level Metaboxes
  for(const Box &box : root.children) {
    if(box.fourcc == FOURCC("moov")) {
      for(const Box &moovChild : box.children) {
        if(moovChild.fourcc == FOURCC("trak")) {
          for(const Box &trakChild : moovChild.children) {
            if(trakChild.fourcc == FOURCC("meta")) {
              metaboxes.push_back(&trakChild);
            }
          }
        }
      }
    }
  }

  return metaboxes;
}

std::vector<const Box *> get_items(const Box &metabox)
{
  std::vector<const Box *> items;

  for(const Box &box : metabox.children) {
    if(box.fourcc == FOURCC("iinf")) {
      for(const Box &infe : box.children) {
        if(infe.fourcc == FOURCC("infe")) {
          items.push_back(&infe);
        }
      }
    }
  }

  return items;
}

std::vector<const Box *> get_image_items(const Box &metabox)
{
  std::vector<const Box *> image_items;
  std::vector<const Box *> all_items = get_items(metabox);

  for(const Box *item : all_items) {

    // Get Item Type
    uint32_t item_type = 0;
    for(const Symbol &sym : item->syms) {
      printf("sym.name: %s, sym.value: %ld\n", sym.name, sym.value);
      if(!sym.name) {
        throw std::runtime_error("Symbol name is null for item");
      }
      if(sym.name && strcmp(sym.name, "item_type") == 0) {
        item_type = static_cast<uint32_t>(sym.value);
        break;
      }
    }
    if(!item_type) {
      throw std::runtime_error("Item type not found for item");
    }

    // clang-format off
    printf("FOURCC(hvc1) = %x\n", FOURCC("hvc1"));
    printf("item_type: %x\n", item_type);
    if(item_type == FOURCC("hvc1") ||
       item_type == FOURCC("unci") ||
       item_type == FOURCC("avc1") ||
       item_type == FOURCC("jpeg") ||
       item_type == FOURCC("j2k1")) {
      // clang-format on
      image_items.push_back(item);
    }
  }

  return image_items;
}

std::vector<const Box *> get_item_properties(const Box &metabox)
{
  std::vector<const Box *> itemProperties;

  for(const Box &box : metabox.children) {
    if(box.fourcc == FOURCC("iprp")) {
      for(const Box &iprpChild : box.children) {
        if(iprpChild.fourcc == FOURCC("ipco")) {
          for(const Box &ipcoChild : iprpChild.children) {
            itemProperties.push_back(&ipcoChild);
          }
        }
      }
    }
  }

  return itemProperties;
}

std::vector<const Box *> get_item_property_associations(const Box &metabox)
{
  std::vector<const Box *> itemPropertyAssociations;

  for(const Box &box : metabox.children) {
    if(box.fourcc == FOURCC("iprp")) {
      for(const Box &iprpChild : box.children) {
        if(iprpChild.fourcc == FOURCC("ipma")) {
          itemPropertyAssociations.push_back(&iprpChild);
        }
      }
    }
  }

  return itemPropertyAssociations;
}

const Symbol &get_symbol_by_name(const Box &box, const char *name)
{

  for(const Symbol &sym : box.syms) {
    if(strcmp(sym.name, name) == 0) {
      return sym;
    }
  }

  throw std::runtime_error("Symbol not found: " + std::string(name));
}