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

#ifndef MEANANDVARIANCEMODULE_H_
#define MEANANDVARIANCEMODULE_H_

#include <string>
#include <vector>
#include <string>
#include <vector>
#include <deque>
#include <node.h>
#include <pthread.h>

namespace rur {

struct Param {
  std::string module_id;
};

class MeanAndVarianceModule : public node::ObjectWrap {
private:
  Param *cliParam;
  
  pthread_t moduleThread;
  bool DestroyFlag;
  pthread_mutex_t destroyMutex;
  
  std::deque<float> readBufDuration;
  float readValDuration;
  pthread_mutex_t readMutexDuration;
  
  std::deque<float> readBufControl;
  float readValControl;
  pthread_mutex_t readMutexControl;
  
  std::deque<float> writeBufMean;
  v8::Persistent<v8::Function> nodeCallBackMean;
  uv_async_t asyncMean;
  pthread_mutex_t writeMutexMean;
  
  std::deque<float> writeBufVariance;
  v8::Persistent<v8::Function> nodeCallBackVariance;
  uv_async_t asyncVariance;
  pthread_mutex_t writeMutexVariance;
  
  static v8::Handle<v8::Value> NodeNew(const v8::Arguments& args);
  
  static v8::Handle<v8::Value> NodeDestroy(const v8::Arguments& args);
  
  bool Destroy();
  
  // Function to be used in NodeJS, not in your C++ code
  static v8::Handle<v8::Value> NodeWriteDuration(const v8::Arguments& args);
  
  // Function to be used in NodeJS, not in your C++ code
  static v8::Handle<v8::Value> NodeWriteControl(const v8::Arguments& args);
  
  // Function to be used in NodeJS, not in your C++ code
  static v8::Handle<v8::Value> NodeRegReadMean(const v8::Arguments& args);
  
  // Function to be used internally, not in your C++ code
  static void CallBackMean(uv_async_t *handle, int status);
  
  // Function to be used in NodeJS, not in your C++ code
  static v8::Handle<v8::Value> NodeRegReadVariance(const v8::Arguments& args);
  
  // Function to be used internally, not in your C++ code
  static void CallBackVariance(uv_async_t *handle, int status);
  
protected:
  static const int channel_count = 4;
  const char* channel[4];
public:
  // Default constructor
  MeanAndVarianceModule();
  
  // Default destructor
  virtual ~MeanAndVarianceModule();
  
  // Extend this with your own code, first call MeanAndVarianceModule::Init(name);
  void Init(std::string& name);
  
  // Function to get Param struct (to subsequently set CLI parameters)
  inline Param *GetParam() { return cliParam; }
  
  // Overwrite this function with your own code
  virtual void Tick() = 0;
  
  // Overwrite this function with your own code
  bool Stop() { return false; }
  
  void Run();
  
  // Function template for NodeJS, do not use in your own code
  static void NodeRegister(v8::Handle<v8::Object> exports);
  
  // Read from this function and assume it means something
  // Remark: check if result is not NULL
  float *readDuration(bool blocking=false);
  
  // Read from this function and assume it means something
  // Remark: check if result is not NULL
  float *readControl(bool blocking=false);
  
  // Write to this function and assume it ends up at some receiving module
  bool writeMean(const float mean);
  
  // Write to this function and assume it ends up at some receiving module
  bool writeVariance(const float variance);
  
};
} // End of namespace

#endif // MEANANDVARIANCEMODULE_H_
