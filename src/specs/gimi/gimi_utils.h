#include "core/fourcc.h"
#include "core/spec.h"

#include <cstring>

std::vector<const Box *> get_all_metaboxes(const Box &root);
std::vector<const Box *> get_items(const Box &metabox);
std::vector<const Box *> get_image_items(const Box &metabox);
std::vector<const Box *> get_ipco_properties(const Box &metabox);
std::vector<const Box *> get_ipma_property_associations(const Box &metabox);
std::vector<const Box *> get_properties_for_item(const Box &metabox, uint32_t item_ID);
const Symbol &get_symbol_by_name(const Box &box, const char *name);

template<typename T = int64_t>
std::vector<T> get_symbol_values(const Box &box, const char *symbol_name);