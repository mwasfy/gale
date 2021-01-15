export C_INCLUDE_PATH=/opt/intel/inteloneapi/compiler/2021.1.1/linux/lib/oclfpga/host/include/
export CPLUS_INCLUDE_PATH=/opt/intel/inteloneapi/compiler/2021.1.1/linux/lib/oclfpga/host/include/
cd ../apps/mis
make
./mis_swi ../../../dataset/ca-coauthors-dblp.mtx ../../../hw/opencl/mis/mis_lu4/bin/mis_lu4.aocx  1 0 2 1
./mis_swi ../../../dataset/socfb-Georgetown15.mtx ../../../hw/opencl/mis/mis_lu4/bin/mis_lu4.aocx  1 0 2 1
./mis_swi ../../../dataset/road-roadNet-CA.mtx ../../../hw/opencl/mis/mis_lu4/bin/mis_lu4.aocx  1 0 2 1
./mis_swi ../../../dataset/eco-everglades.edges ../../../hw/opencl/mis/mis_lu4/bin/mis_lu4.aocx  1 0 2 1
./mis_swi ../../../dataset/eco-foodweb-baydry.edges ../../../hw/opencl/mis/mis_lu4/bin/mis_lu4.aocx  1 0 2 1
./mis_swi ../../../dataset/web-wikipedia2009.mtx ../../../hw/opencl/mis/mis_lu4/bin/mis_lu4.aocx  1 0 2 1
./mis_swi ../../../dataset/eco-stmarks.edges ../../../hw/opencl/mis/mis_lu4/bin/mis_lu4.aocx  1 0 2 1
./mis_swi ../../../dataset/socfb-Duke14.mtx ../../../hw/opencl/mis/mis_lu4/bin/mis_lu4.aocx  1 0 2 1
