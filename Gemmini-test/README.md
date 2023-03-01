# Gemmini RoCC accelerator
Gemmini is a side project of chipyard, and it working mechanism is load the NN model into RoCC accelerator, although it couldn't contain lot of PE (processing element) of systolic array, but it have a great advantage in boost because it accelerator is nearly the CPU, and it could share memory with L2-cache. Above descrption, this architecture are fitting edge computation device.
(This tutorial use absolutely directory, if you pretty sure where it is, you can just `cd` to the directory)
1. Chipyard & esp-tools installation
2. Setting up gemmini workspace
3. Project: Complex multiplication
4. Project: Running onnx inference model

## Complementary
* linux version
* Chipyard dependency
  conda 


## 1. Chipyard & esp-tools installation
First, clone the repository from `ucb-bar/chipyard`
and build conda environment, then build esp-tools(This toolchain is for Hwacha and Gemmini to use).
(If there have any problem with conda development, see ![here](https://chipyard.readthedocs.io/en/stable/Chipyard-Basics/Initial-Repo-Setup.html))
```
git clone https://github.com/ucb-bar/chipyard.git
wget "https://github.com/conda-forge/miniforge/releases/latest/download/Mambaforge-$(uname)-$(uname -m).sh"
bash Mambaforge-$(uname)-$(uname -m).sh
source  ~/mambaforge/etc/profile.d
conda activate base

conda install -n base conda-lock
cd chipyard
git checkout 1.8.1
./build-setup.sh esp-tools
```
Toolchain is setup already, then get into conda environment and install `spike`, this tool is a RISC-V instruction accurate simulator, It also could be extended gemmini instructions(we don't need MIDAS simulation recently).
```
source env.sh
# (/home/user/chipyard/.conda-env) user@user:~/chipyard

cd generators/gemmini
git config remote.origin.fetch "+refs/heads/*:refs/remotes/origin/*"
git fetch && git checkout v0.7.0
git submodule update --init --recursive

SPIKE_HASH=$(cat SPIKE.hash)

cd -
cd toolchains/esp-tools/riscv-isa-sim/build
git fetch && git checkout $SPIKE_HASH
make && make install

```
If you are encounter the following issue, it means that **your machine does not have enough memory** to run this command, closed other applications would solve this problem (required 8GB RAM).

see issue: https://github.com/ucb-bar/gemmini/issues/239

## 2. Setting up gemmini workspace
**Setting up gemmini configuration files and other subdirections**, initialized all Gemmini materials.
```
cd ~/chipyard/generators/gemmini
./scripts/setup-paths.sh
```

**Building software of compile programs**, including large DNN models like ResNet50, and small matrix-multiplication tests.
```
cd ~/chipyard/generators/gemmini/software/gemmini-rocc-tests
./build.sh

# Ignore `[error] Picked up JAVA_TOOL_OPTIONS: ...`
```
After these step, Take a look of some important directories that could be used.
```
generators/gemmini/
 |-> config/    # Customized of your own design (CPU, Gemmini, SoC configuration)
 |-> scripts/   # Runnig your configuration or simulation (spike, verilator, build-onnx)
 |-> software/  # Gemmini's programs (RoCC-test, onnxruntime etc.)
 |-> src/       # Gemmini's submodules (PE, Arithmetic etc.)
```
Following image is simulators functionals and features.

![Simulators](https://i.imgur.com/pO5hba5.png)

**Building Hardware and Cycle-Accurate Simulator**, this step would generate the verilog code and corresponding simulator source (Verilator).
```
cd ~/chipyard/generators/gemmini/
./scripts/build-verilator --debug 
# --debug would generate the .vcs file at /waveforms

# Build a directory /generated-src/, you'll able to find Verilog description of Customized SoC.
# Include these files: *top.v, Sim*.v ......and so on.
```
**Building Functional Simulators**, generate a functional ISA simulator (Spike).
```
cd chipyard/generators/gemmini
./scripts/build-spike.sh
```
**Running Basic Simulators**, this step could test the functional of your all simulators.
```
cd chipyard/generators/gemmini

# Run a large DNN workload in the functional simulator
./scripts/run-spike.sh resnet50

# Run a smaller workload in baremetal mode, on a cycle-accurate simulator
./scripts/run-verilator.sh template

# Run a smaller workload with the proxy-kernel, on a cycle accurate simulator
./scripts/run-verilator.sh --pk template

# Or, if you want to generate waveforms in `waveforms/`:
# ./scripts/run-verilator.sh --pk --debug template
```

If you want to learn more detail about gemmini, please see [here](https://github.com/ucb-bar/gemmini).

## 3. Project: Complex multiplication
Full tutorial on YouTube: https://www.youtube.com/watch?v=Q6gfthExSts&t, **This tutorial also contains next project (onnx model)**, We suggested that you could follow this video step by step, and the slido are also with the repository: `/tutorial/Gemmini_full_tutorial.pdf`.

Here is `configs/` correspoding function.
```
gemmini/configs/
 |-> CPUConfigs.scala             # Selected your own cpu collections 
 |-> GemminiCustomConfigs.scala   # Customized your config (Add Complex arithmetic)
 |-> GemminiDefaultConfigs.scala  # Default setting config 
 |-> SoCConfigs.scala             # SoC build rule could designed yourself
```
In order to make a functional of Complex multiplication, we need to define the data type of complex and it arithmetic mechanisms.
```
git clone https://github.com/NinoX-FPGA/Chipyard-for-FPGA.git
REPO=~/Chipyard-for-FPGA/Gemmini-test/
cd $REPO/gemmini/src/main
cp Aritmetic.scala ~/chipyard/generators/gemmini/src/main/gemmini/Aritmetic.scala  # To replace the origin file.

# If you want to take a look of what differents, you can open Arithmetic.scala and Ctrl+F James add.
```
After added the new mechanism of arithmetic, change your configuration of `GemminiCustomConfigs.scala`
```
cd ~/chipyard/generators/gemmini/configs/
nano GemminiCustomConfigs.scala
```
```scala
/* # GemminiCustomConfigs.scala */
...
object GemminiCustomConfigs{
    ...
    val ibertInferenceConfig=defaultConfig.copy(...)
    // Add
    val complexConfig = GemminiArrayConfig[Complex, Float, Float](
        inputType = new Complex(16),
        accType = new Complex(16),
        
        spatialArrayOutputType = new Complex(16)
    )
    // End add
    val customConfig = complexConfig  // <- change here
}
...
```
Build your change of the hardware, and copy the simulation programs into corresponding directory.
```
cd ~/chipyard/generators/gemmini
./scripts/build-verilator.sh
cp -f $REPO/software/gemmini-rocc-tests/bareMetalC/complex_mul.c Makefile ~/chipyard/generators/gemmini/software/gemmini-rocc-tests/bareMetalC/
cp -f $REPO/software/gemmini-rocc-tests/include ~/chipyard/generators/gemmini/software/gemmini-rocc-tests/include/

# If you want to take a look of what differents, you can open following file and Ctrl+F James add:
    |-> bareMetalC/complex_mul.c
    |-> bareMetalC/Makefile
    |-> include/gemmini_params.h
    |-> include/gemmini_testutils.h

cd ~/chipyard/generators/gemmini/software/gemmini-rocc-tests/
./build.sh
```
Now you have tests file, then run Complex-Mult simulator, in this case, the result will show In1 and In2 are also ***diag([0 + 1i])***, and Out is equal to ***diag([-1 + 0i])***.
```
cd ../..
./scripts/run-verilator.sh complex_mul

# This remote compiled with JTAG........
#[UART] UART0 is here(stdin/stdout).
.....
```
## 4. Project: Running onnx inference model
In this case, we use basic `onnx model` to run **Image recognition**, and the following model is [here](
https://tinyurl.com/gemm-iiswc), you can also check the structure in [netron.app](https://netron.app).

This section would not run onnx-training, because the material doesn't enough in tutorial, also for writer's opinion, in PyTorch training is more powerful than onnx-training, so we demostration onnx-inference model here.

First, initialized the `software/onnxruntime-riscv` , and make`ort_test`.
```
cd ~/chipyard/generators/gemmini/software/onnxruntime-riscv
./build.sh # It will take long
cd systolic_runner/imagenet_runner/
./build.sh

# ort_test: A simulation processor (binary file)
```

Try of following command, you may get following error 
(If doesn't, It means they fixed there repositories).
```
spike pk ort_test

# bbl loader; bad syscall #98

spike --extension=gemmini pk ort_test -h

# couldn't find extension "gemmini" in share library 'libcustomext.so'

```
Solution of the first error is edit the proxy kernel file inside the `~/chipyard/toolchains/esp-tools/riscv-pk/pk` to fix syscall 98 error, edit `syscall.h` and `syscall.c` of following programming
After fixed it, you will show help options listed on terminal`onnx model runner:....`.
```
cp -f $REPO/toolchains/riscv-pk/syscall.c syscall.h ~/chipyard/toolchains/esp-tools/riscv-pk/pk/
# Add the definition of syscall 98.
cd /chipyard/toolchains/esp-tools/riscv-pk/build
make
```

For second issue, take a look [here](https://github.com/hust-gaoyujing/routine-21/blob/d4e34c140ac45b3122f9460e79ba3d099839f211/inferencor/gemmini_code/gemmini-mlsys-tutorial-2022/tutorial/checkpoint.sh) at the `build-onnx-inference` section, following the inside commands, in this repository, you will need the material of `gemmini_params_int.h` to define your input datatype
```
cp $REPO/gemmini/tutorial/gemmini_params_int.h ~/chipyard/toolchains/esp-tools/riscv-isa-sim/gemmini/gemmini_params.h
cd ../../toolchains/esp-tools/riscv-isa-sim/build
```
Before makefile, there has somewhere you need to correct `riscv-isa-sim/gemmini/gemmini.cc`:
``` cpp
/* # ~/chipyard/toolchains/esp-tools/riscv-isa-sim/gemmini/gemmini.cc */
...
#if defined(HAS_MVIN_SCALE) || defined(HAS_MVIN_ACC_SCALE)
scale_t_bits gemmini_t::acc_scale_t_to_acc_scale_t_bits(scale_t scale){

                     // ^^^ change: acc_scale_t_to_acc_scale_t_bits-> scale_t_to_scale_t_bits
    union{
        scale_t scale;
        scale_t_bits bits;
    } un;
    return un.bits;
}
...
#endif
```
Then make with `Makefile`:
```
make & make install
```
Next, run simulation of onnx model in berkeley bootloader, if you don't want to type command so many times, you can create a script to run it automatically:

```
cd  ~/chipyard/generator/gemmini/scripts
nano run-onnx-inference.sh

# Add commands in script
cd ./software/onnxruntime-riscv/
cd ./systolic_runner/imagenet_runner/

spike --extension=gemmini pk ort_test -m resnet50_opt_quant.onnx -i images/dog.jpg -p caffe2 -x 2 -O 99
# End script

cd .. # It's necessary, or it would not be work in this directory.
./scripts/run-onnx-inference.sh
```
## (Next) Check design area of zedboard in vivado
(**Researching...**)
Sim**.v -> Sim**.sv (because some error would be reported in verilog)

Customize Makefrag to build bitstream into image file
```
git clone https://github.com/<repository_name>.git

```

## Resources

Chipyard Stable Documentation: https://chipyard.readthedocs.io/

Chipyard (x FireSim) Tutorial: https://fires.im/tutorial

Chipyard Basics slides: https://fires.im/isca22-slides-pdf/02_chipyard_basics.pdf

Chipyard Tutorial Exercise slides: https://fires.im/isca22-slides-pdf/03_building_custom_socs.pdf

