#ifndef __VX_KHR_COMPATIBLE_H__
#define __VX_KHR_COMPATIBLE_H__
/*
 VX_DECONVOLUTION_WEIGHT_LAYOUT_COMPATIBLE_KHRONOS is used to distingush deconvolution weight layout
 [value]
 0: weight_layout is whnc
 1: weight_layout is whcn
*/
#define VX_DECONVOLUTION_WEIGHT_LAYOUT_COMPATIBLE_KHRONOS 1
/*
 VX_CONVERT_POLICY_WRAP_ENABLE is used to differentiate two overflow_policys(VX_CONVERT_POLICY_WRAP and VX_CONVERT_POLICY_SAT)
 [value]
 0: both overflow_policys considered as VX_CONVERT_POLICY_SAT
 1: overflow_policy is determined by arguments.
*/
#define VX_CONVERT_POLICY_WRAP_ENABLE 1

#endif /* __VX_KHR_COMPATIBLE_H__ */