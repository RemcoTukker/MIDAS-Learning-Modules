#define BUILDING_NODE_EXTENSION
#include <node.h>
#include <MeanAndVarianceModuleExt.h>

using namespace v8;
using namespace rur;

void RegisterModule(Handle<Object> exports) {
  MeanAndVarianceModuleExt::NodeRegister(exports);
}

NODE_MODULE(MeanAndVarianceModule, RegisterModule)
