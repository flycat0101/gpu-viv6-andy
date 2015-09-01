1. gc_vsc_drvi_interface.h is the header that driver needs include, it will be moved into ./drvi later
2. Under ./drvi, following 2 files need to be removed into HAL level of driver later, compiler only generates them, not maintain/use them. Driver will use them to do HW-linkage/programing and other API related functions. Now they are temporarily put here.
   gc_vsc_drvi_shader_profile.h
   gc_vsc_drvi_program_profile.h