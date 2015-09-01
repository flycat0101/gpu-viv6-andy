vsc is abbre of vivante shader compiler (or codegen). It includes following subsections.

1. include
It includes all header files VSC needs, sub-directory hierarchy should be same as hierarchy of .c implementation

2. old_impl
It includes original implementation (all use gcsl IR and pattern-driven codegen is used), all these files are stuffed into this directory. It does not access any other directory which are for new implementation of vir, except that calling lower function to lower current gcsl IR to low level vir IR. In future, it will be fully removed when vir becomes matural.

3. drvi
It includes implemenation to commnucate with outside world, mainly for driver.

4. utils
It includes all basic data structures and their operations,such as link (bi-link)/DAG/DG/AG/Hash/Tree/bit-vector, etc. Note it must be IR independent.

5. vir
It includes ir itself and all analysises and opts based on vir. So it is the key part of backend. It has several sub-directories, such as ir/analysis/transform/codegen/... Note that all machine-independent opts are put under 'transform', and all machine-dependent opts are put under 'codegen'.

6. chip
It includes all HW-arch related info, so it will be independent on VIR. Note due to state programming is responsibility of driver, so chip/gc_vsc_chip_state_programming.c will be removed to driver side later along with include\drvi\gc_vsc_drvi_shader_profile.h and gc_vsc_drvi_program_profile.h
