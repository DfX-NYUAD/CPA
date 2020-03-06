date
set_host_options -max_cores 4
set compile_seqmap_propagate_constants     false
set compile_seqmap_propagate_high_effort   false
set compile_enable_register_merging        false
set write_sdc_output_net_resistance        false
set timing_separate_clock_gating_group     true
set verilogout_no_tri tru
set html_log_enable true

set design   [getenv "DESIGN"]
set prj_name [getenv "PROJECT_NAME"]
set prj_path [getenv "PROJECT_MODULES"]
set target_lib   [getenv "SYNTH_LIBRARY"]
set clkname  [getenv "CLKNAME"]
set work_dir  [getenv "SYNTH_RUN"]

#if {[file exist $work_dir]} {
#sh rm -rf $work_dir
#}

sh mkdir -p $work_dir/reports
sh mkdir -p $work_dir/netlist

set search_path [concat * $search_path]

sh rm -rf ./work
define_design_lib WORK -path ./work

  set_svf $design.svf

  set target_library $target_lib

  set link_library $target_lib

source -echo -verbose ./analyze.tcl

elaborate $design
date
 

link

  set_max_area 0
  set_clock_gating_style -sequential_cell latch -positive_edge_logic {nand} -negative_edge_logic {nor} -minimum_bitwidth 5 -max_fanout 64


  source -echo -verbose $prj_path/$prj_name/synth/constraints.tcl


  group_path -name output_group -to   [all_outputs]
  group_path -name input_group  -from [all_inputs]

date
mem -all -verbose
  compile_ultra -no_autoungroup -no_seq_output_inversion -no_boundary_optimization
date
mem -all -verbose
  optimize_netlist -area
date
mem -all -verbose
  compile_ultra -no_autoungroup -no_seq_output_inversion -no_boundary_optimization -incremental
date
mem -all -verbose

   change_names -hier -rule verilog

   write_file -hierarchy -format verilog -output "$work_dir/netlist/$design.v"
   write_sdc "$work_dir/netlist/$design.sdc"
   ungroup -all -flatten
   write_file -hierarchy -format verilog -output "$work_dir/netlist/${design}_flat.v"

date
exit
