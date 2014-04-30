/**
 * This file is created at Almende B.V. It is open-source software and part of the Common 
 * Hybrid Agent Platform (CHAP). A toolbox with a lot of open-source tools, ranging from 
 * thread pools and TCP/IP components to control architectures and learning algorithms. 
 * This software is published under the GNU Lesser General Public license (LGPL).
 *
 * It is not possible to add usage restrictions to an open-source license. Nevertheless,
 * we personally strongly object against this software being used by the military, in the
 * bio-industry, for animal experimentation, or anything that violates the Universal
 * Declaration of Human Rights.
 *
 * @author               Remco Tukker
 * @copyright            Almende
 * @date                 Apr 28, 2014
 * @license              
 */

#include "MeanAndVarianceModule.h"
#include <MeanAndVarianceModuleExt.h>

namespace rur {
using namespace v8;

MeanAndVarianceModule::MeanAndVarianceModule():
  cliParam(0)
{
  const char* const channel[4] = {"readDuration", "readControl", "writeMean", "writeVariance"};
  cliParam = new Param();
  DestroyFlag = false;
  readBufDuration = std::deque<float>(0);
  readValDuration = float(0);
  readBufControl = std::deque<float>(0);
  readValControl = float(0);
  writeBufMean = std::deque<float>(0);
  writeBufVariance = std::deque<float>(0);
}

MeanAndVarianceModule::~MeanAndVarianceModule() {
  delete cliParam;
}

void MeanAndVarianceModule::Init(std::string & name) {
  cliParam->module_id = name;
  
}

static void* RunModule(void* object) {
  static_cast<MeanAndVarianceModuleExt*>(object)->Run();
}

void MeanAndVarianceModule::Run() {
  while (true) {
    Tick();
  }
}

v8::Handle<v8::Value> MeanAndVarianceModule::NodeNew(const v8::Arguments& args) {
  v8::HandleScope scope;
  if ((args.Length() < 1) || (!args[0]->IsString())) {
    return v8::ThrowException(v8::String::New("Invalid argument, must provide a string."));
  }
  MeanAndVarianceModuleExt* obj = new MeanAndVarianceModuleExt();
  obj->Wrap(args.This());
  
  std::string name = (std::string)*v8::String::AsciiValue(args[0]);
  obj->Init(name);
  
  pthread_mutex_init(&(obj->destroyMutex), NULL);
  
  // Init ports
  pthread_mutex_init(&(obj->readMutexDuration), NULL);
  
  pthread_mutex_init(&(obj->readMutexControl), NULL);
  
  pthread_mutex_init(&(obj->writeMutexMean), NULL);
  uv_async_init(uv_default_loop() , &(obj->asyncMean), &(obj->CallBackMean));
  
  pthread_mutex_init(&(obj->writeMutexVariance), NULL);
  uv_async_init(uv_default_loop() , &(obj->asyncVariance), &(obj->CallBackVariance));
  
  // Start the module loop
  pthread_create(&(obj->moduleThread), 0, RunModule, obj);
  
  // Make this object persistent
  obj->Ref();
  
  return args.This();
}

v8::Handle<v8::Value> MeanAndVarianceModule::NodeDestroy(const v8::Arguments& args) {
  v8::HandleScope scope;
  MeanAndVarianceModuleExt* obj = ObjectWrap::Unwrap<MeanAndVarianceModuleExt>(args.This());
  return scope.Close(v8::Boolean::New(obj->Destroy()));
}

bool MeanAndVarianceModule::Destroy() {
  bool canDestroy = true;
  if (canDestroy) {
    pthread_mutex_lock(&writeMutexMean);
    if (!writeBufMean.empty())
      canDestroy = false;
    pthread_mutex_unlock(&writeMutexMean);
  }
  if (canDestroy) {
    pthread_mutex_lock(&writeMutexVariance);
    if (!writeBufVariance.empty())
      canDestroy = false;
    pthread_mutex_unlock(&writeMutexVariance);
  }
  if (canDestroy) {
    pthread_cancel(moduleThread);
    Unref();
    return true;
  }
  else {
    pthread_mutex_lock(&destroyMutex);
    DestroyFlag = true;
    pthread_mutex_unlock(&destroyMutex);
    return true; // return true anyway?
  }
}

void MeanAndVarianceModule::NodeRegister(v8::Handle<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(NodeNew);
  tpl->SetClassName(v8::String::NewSymbol("MeanAndVarianceModule"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  
  // Prototypes
  tpl->PrototypeTemplate()->Set(v8::String::NewSymbol("Destroy"), v8::FunctionTemplate::New(NodeDestroy)->GetFunction());
  tpl->PrototypeTemplate()->Set(v8::String::NewSymbol("WriteDuration"), v8::FunctionTemplate::New(NodeWriteDuration)->GetFunction());
  tpl->PrototypeTemplate()->Set(v8::String::NewSymbol("WriteControl"), v8::FunctionTemplate::New(NodeWriteControl)->GetFunction());
  tpl->PrototypeTemplate()->Set(v8::String::NewSymbol("RegReadMean"), v8::FunctionTemplate::New(NodeRegReadMean)->GetFunction());
  tpl->PrototypeTemplate()->Set(v8::String::NewSymbol("RegReadVariance"), v8::FunctionTemplate::New(NodeRegReadVariance)->GetFunction());
  
  v8::Persistent<v8::Function> constructor = v8::Persistent<v8::Function>::New(tpl->GetFunction());
  exports->Set(v8::String::NewSymbol("MeanAndVarianceModule"), constructor);
}

v8::Handle<v8::Value> MeanAndVarianceModule::NodeWriteDuration(const v8::Arguments& args) {
  v8::HandleScope scope;
  MeanAndVarianceModuleExt* obj = ObjectWrap::Unwrap<MeanAndVarianceModuleExt>(args.This());
  if (args.Length() < 1)
    return scope.Close(v8::Boolean::New(false)); // Could also throw an exception
  pthread_mutex_lock(&(obj->readMutexDuration));
  obj->readBufDuration.push_back(args[0]->NumberValue());
  pthread_mutex_unlock(&(obj->readMutexDuration));
  return scope.Close(v8::Boolean::New(true));
}

float* MeanAndVarianceModule::readDuration(bool blocking) {
  pthread_mutex_lock(&destroyMutex);
  bool destroy = DestroyFlag;
  pthread_mutex_unlock(&destroyMutex);
  if (destroy)
    return NULL;
  pthread_mutex_lock(&readMutexDuration);
  if (readBufDuration.empty()) {
    pthread_mutex_unlock(&readMutexDuration); // Don't forget to unlock!
    return NULL;
  }
  readValDuration = readBufDuration.front();
  readBufDuration.pop_front();
  pthread_mutex_unlock(&readMutexDuration);
  return &readValDuration;
}

v8::Handle<v8::Value> MeanAndVarianceModule::NodeWriteControl(const v8::Arguments& args) {
  v8::HandleScope scope;
  MeanAndVarianceModuleExt* obj = ObjectWrap::Unwrap<MeanAndVarianceModuleExt>(args.This());
  if (args.Length() < 1)
    return scope.Close(v8::Boolean::New(false)); // Could also throw an exception
  pthread_mutex_lock(&(obj->readMutexControl));
  obj->readBufControl.push_back(args[0]->NumberValue());
  pthread_mutex_unlock(&(obj->readMutexControl));
  return scope.Close(v8::Boolean::New(true));
}

float* MeanAndVarianceModule::readControl(bool blocking) {
  pthread_mutex_lock(&destroyMutex);
  bool destroy = DestroyFlag;
  pthread_mutex_unlock(&destroyMutex);
  if (destroy)
    return NULL;
  pthread_mutex_lock(&readMutexControl);
  if (readBufControl.empty()) {
    pthread_mutex_unlock(&readMutexControl); // Don't forget to unlock!
    return NULL;
  }
  readValControl = readBufControl.front();
  readBufControl.pop_front();
  pthread_mutex_unlock(&readMutexControl);
  return &readValControl;
}

v8::Handle<v8::Value> MeanAndVarianceModule::NodeRegReadMean(const v8::Arguments& args) {
  v8::HandleScope scope;
  MeanAndVarianceModuleExt* obj = ObjectWrap::Unwrap<MeanAndVarianceModuleExt>(args.This());
  if (args.Length() < 1 || !args[0]->IsFunction())
    return scope.Close(v8::Boolean::New(false)); // Could also throw an exception
  v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(args[0]);
  obj->nodeCallBackMean = v8::Persistent<v8::Function>::New(callback);
  return scope.Close(v8::Boolean::New(true));
}

void MeanAndVarianceModule::CallBackMean(uv_async_t *handle, int status) {
  v8::HandleScope scope;
  MeanAndVarianceModuleExt* obj = (MeanAndVarianceModuleExt*)(handle->data);
  const unsigned argc = 1;
  while (true) {
    pthread_mutex_lock(&(obj->writeMutexMean));
    if (obj->writeBufMean.empty()) {
      pthread_mutex_unlock(&(obj->writeMutexMean)); // Don't forget to unlock!
      break;
    }
    v8::Local<v8::Value> argv[argc] = { v8::Local<v8::Value>::New(v8::Number::New(obj->writeBufMean.front())) };
    obj->writeBufMean.pop_front();
    pthread_mutex_unlock(&(obj->writeMutexMean));
    if (!obj->nodeCallBackMean.IsEmpty())
      obj->nodeCallBackMean->Call(v8::Context::GetCurrent()->Global(), argc, argv);
  }
  pthread_mutex_lock(&(obj->destroyMutex));
  bool destroy = obj->DestroyFlag;
  pthread_mutex_unlock(&(obj->destroyMutex));
  if (destroy)
    obj->Destroy();
}

bool MeanAndVarianceModule::writeMean(const float mean) {
  pthread_mutex_lock(&destroyMutex);
  bool destroy = DestroyFlag;
  pthread_mutex_unlock(&destroyMutex);
  if (destroy)
    return false;
  pthread_mutex_lock(&writeMutexMean);
  writeBufMean.push_back(mean);
  pthread_mutex_unlock(&writeMutexMean);
  asyncMean.data = (void*) this;
  uv_async_send(&asyncMean);
  return true;
}

v8::Handle<v8::Value> MeanAndVarianceModule::NodeRegReadVariance(const v8::Arguments& args) {
  v8::HandleScope scope;
  MeanAndVarianceModuleExt* obj = ObjectWrap::Unwrap<MeanAndVarianceModuleExt>(args.This());
  if (args.Length() < 1 || !args[0]->IsFunction())
    return scope.Close(v8::Boolean::New(false)); // Could also throw an exception
  v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(args[0]);
  obj->nodeCallBackVariance = v8::Persistent<v8::Function>::New(callback);
  return scope.Close(v8::Boolean::New(true));
}

void MeanAndVarianceModule::CallBackVariance(uv_async_t *handle, int status) {
  v8::HandleScope scope;
  MeanAndVarianceModuleExt* obj = (MeanAndVarianceModuleExt*)(handle->data);
  const unsigned argc = 1;
  while (true) {
    pthread_mutex_lock(&(obj->writeMutexVariance));
    if (obj->writeBufVariance.empty()) {
      pthread_mutex_unlock(&(obj->writeMutexVariance)); // Don't forget to unlock!
      break;
    }
    v8::Local<v8::Value> argv[argc] = { v8::Local<v8::Value>::New(v8::Number::New(obj->writeBufVariance.front())) };
    obj->writeBufVariance.pop_front();
    pthread_mutex_unlock(&(obj->writeMutexVariance));
    if (!obj->nodeCallBackVariance.IsEmpty())
      obj->nodeCallBackVariance->Call(v8::Context::GetCurrent()->Global(), argc, argv);
  }
  pthread_mutex_lock(&(obj->destroyMutex));
  bool destroy = obj->DestroyFlag;
  pthread_mutex_unlock(&(obj->destroyMutex));
  if (destroy)
    obj->Destroy();
}

bool MeanAndVarianceModule::writeVariance(const float variance) {
  pthread_mutex_lock(&destroyMutex);
  bool destroy = DestroyFlag;
  pthread_mutex_unlock(&destroyMutex);
  if (destroy)
    return false;
  pthread_mutex_lock(&writeMutexVariance);
  writeBufVariance.push_back(variance);
  pthread_mutex_unlock(&writeMutexVariance);
  asyncVariance.data = (void*) this;
  uv_async_send(&asyncVariance);
  return true;
}

} // namespace
