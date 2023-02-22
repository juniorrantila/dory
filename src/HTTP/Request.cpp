#include "Request.h"

namespace HTTP {

StringView Method::name() const
{
    switch(m_type) {
        case Get: return "GET"sv;
        case Post: return "POST"sv;
    }
}

}
