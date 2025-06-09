#include "guid.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace SGIS{

string Guid::CreateGuid(){
    boost::uuids::uuid uid = boost::uuids::random_generator()();
   return boost::uuids::to_string(uid);
}

}
