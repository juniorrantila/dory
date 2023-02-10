#include "Response.h"

namespace HTTP {

StringView response_code_string(ResponseCode code)
{
    switch(code) {
    case ResponseCode::Continue: return "Continue"sv;
    case ResponseCode::Ok: return "OK"sv;
    case ResponseCode::NotFound: return "Not Found"sv;
    case ResponseCode::InternalServerError: return "Internal Server Error"sv;
    }
}

}
