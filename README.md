# signal_impact_test
Code to test the impact of signals on a process (sent to process vs. sent to thread)

The supplied code is supposed to help testing whether certain signals are caught and stop either:
  1.  The current thread only
  2.  The current process
  3.  The entire current process group
