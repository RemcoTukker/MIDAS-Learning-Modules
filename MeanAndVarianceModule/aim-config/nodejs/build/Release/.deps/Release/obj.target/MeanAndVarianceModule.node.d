cmd_Release/obj.target/MeanAndVarianceModule.node := flock ./Release/linker.lock g++ -shared -pthread -rdynamic -m64 -pthread  -Wl,-soname=MeanAndVarianceModule.node -o Release/obj.target/MeanAndVarianceModule.node -Wl,--start-group Release/obj.target/MeanAndVarianceModule/../../aim-core/src/MeanAndVarianceModule.o Release/obj.target/MeanAndVarianceModule/MeanAndVarianceModuleNode.o Release/obj.target/MeanAndVarianceModule/../../src/MeanAndVarianceModuleExt.o -Wl,--end-group 