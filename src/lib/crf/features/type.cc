#include "base.h"
#include "crf/features/type.h"

namespace NLP { namespace CRF {

const Type Types::words = {"word features", "w", 0};
const Type Types::pos = {"pos features", "p", 1};
const Type Types::nextword = {"next word feature", "nw", 2};
const Type Types::nextnextword = {"nextnext word feature", "nnw", 3};
const Type Types::prevword = {"prev word feature", "pw", 4};
const Type Types::prevprevword = {"prev prev word feature", "ppw", 5};
const Type Types::nextpos = {"next pos feature", "np", 6};
const Type Types::nextnextpos = {"nextnext pos feature", "nnp", 7};
const Type Types::prevpos = {"prev pos feature", "pp", 8};
const Type Types::prevprevpos = {"prev prev pos feature", "ppp", 9};

} }
