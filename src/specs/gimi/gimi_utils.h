#include "core/fourcc.h"
#include "core/spec.h"

#include <cstring>

std::vector<const Box *> get_all_metaboxes(const Box &root);
std::vector<const Box *> get_items(const Box &metabox);
std::vector<const Box *> get_image_items(const Box &metabox);
std::vector<const Box *> get_item_properties(const Box &metabox);
std::vector<const Box *> get_item_property_associations(const Box &metabox);
const Symbol &get_symbol_by_name(const Box &box, const char *name);