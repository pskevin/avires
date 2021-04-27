#include "page.h"


map<Page::Type, uint64_t> Page::Size = Page::PopulateSize();
map<Page::Type, uint64_t> Page::OffsetMask = Page::PopulateOffsetMask();
map<Page::Type, uint64_t> Page::FrameNumberMask = Page::PopulateFrameNumberMask();