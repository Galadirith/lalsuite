"""
  Classes needed for the time domain pulsar pipeline.

  This has been greatly inspired (well blatently hacked together from) the stochastic pipeline by
  Adam Mercer and the frequency domain binary pulsar search by Chris Messenger.
  
  Matt Pitkin 30/03/06
  
"""

__author__ = 'Matt Pitkin <matthew.pitkin@glasgow.ac.uk>'
__date__ = '$Date$'
__version__ = '$Revision$'

import string
import exceptions
import os
from glue import pipeline

# And begin ...

class heterodyneJob(pipeline.CondorDAGJob, pipeline.AnalysisJob):
  """
  A lalapps_heterodyne_pulsar job to coarse heterodyne the data.
  """
  def __init__(self,execu,univ='standard'):
    self.__executable = execu # gets the executable from .ini
    self.__universe = univ
    pipeline.CondorDAGJob.__init__(self, self.__universe, self.__executable)
    pipeline.AnalysisJob.__init__(self, None)
       
    # set log files for job
    self.set_stdout_file('logs/heterodyne_pulsar-$(cluster).out')
    self.set_stderr_file('logs/heterodyne_pulsar-$(cluster).err')
    self.set_sub_file('heterodyne_pulsar.sub')

class heterodyneNode(pipeline.CondorDAGNode, pipeline.AnalysisNode):
  """
  A heterodyneNode runs an instance of lalapps_heterodyne_pulsar in coarse heterodyne mode in
  a condor DAG.
  """
  def __init__(self,job):
    """
    job = A CondorDAGJob that can run an instance of lalapps_CalculateSensitivity.
    """
    pipeline.CondorDAGNode.__init__(self,job)
    pipeline.AnalysisNode.__init__(self)
    
    # initilise job variables
    self.__ifo = None
    self.__param_file = None
    self.__freq_factor = None
    self.__filter_knee = None
    self.__sample_rate = None
    self.__resample_rate = None
    self.__data_file = None
    self.__channel = None
    self.__seg_list = None
    self.__data_file = None
    self.__output_dir = None
    self.__het_flag = None
    self.__pulsar = None
    self.__high_pass = None
    self.__scale_fac = None
    self.__manual_epoch = None
      
  def set_data_file(self,data_file):
    # set file containing data to be heterodyne (either list of frames or coarse het output)
    self.add_var_opt('data-file',data_file)
    self.__data_file = data_file
    
  def set_output_dir(self,output_dir):
    # set output directory
    self.add_var_opt('output-dir',output_dir)
    self.__output_dir = output_dir
    
  def set_seg_list(self,seg_list):
    # set segment list to be used
    self.add_var_opt('seg-file',seg_list)
    self.__seg_list = seg_list
    
  def set_ifo(self, ifo):
    # set detector
    self.add_var_opt('ifo', ifo)
    self.__ifo = ifo
    
  def set_param_file(self, param_file):
    # set pulsar parameter file
    self.add_var_opt('param-file', param_file)
    self.__param_file = param_file
  
  def set_freq_factor(self, freq_factor):
    # set the factor by which to muliply the pulsar spin frequency (normally 2.0)
    self.add_var_opt('freq-factor', freq_factor)
    self.__freq_factor = freq_factor
    
  def set_param_file_update(self, param_file_update):
    # set file containing updated pulsar parameters
    self.add_var_opt('param-file-update',param_file_update)
    
  def set_manual_epoch(self, manual_epoch):
      # set manual pulsar epoch
      self.add_var_opt('manual-epoch',manual_epoch)
      self.__manual_epoch = manual_epoch
    
  def set_ephem_earth_file(self, ephem_earth_file):
    # set the file containing the earth's ephemeris
    self.add_var_opt('ephem-earth-file', ephem_earth_file)
    
  def set_ephem_sun_file(self, ephem_sun_file):
    # set the file containing the sun's ephemeris
    self.add_var_opt('ephem-sun-file', ephem_sun_file)

  def set_ephem_time_file(self, ephem_time_file):
    # set the file containing the Einstein delay correction ephemeris
    self.add_var_opt('ephem-time-file', ephem_time_file)
    
  def set_pulsar(self,pulsar):
    # set pulsar name
    self.add_var_opt('pulsar',pulsar)
    self.__pulsar = pulsar
    
  def set_het_flag(self,het_flag):
    # set heterodyne flag
    self.add_var_opt('heterodyne-flag',het_flag)
    self.__het_flag = het_flag
    
  def set_filter_knee(self,filter_knee):
    # set filter knee frequency
    self.add_var_opt('filter-knee',filter_knee)
    self.__filter_knee = filter_knee
  
  def set_channel(self,channel):
    # set channel containing data from frames
    self.add_var_opt('channel',channel)
    self.__channel = channel
    
  def set_sample_rate(self,sample_rate):
    # set sample rate of input data
    self.add_var_opt('sample-rate',sample_rate)
    self.__sample_rate = sample_rate
    
  def set_resample_rate(self,resample_rate):
    # set resample rate for output data
    self.add_var_opt('resample-rate',resample_rate)
    self.__resample_rate = resample_rate
    
  def set_stddev_thresh(self,stddev_thresh):
    # set standard deviation threshold at which to remove outliers
    self.add_var_opt('stddev-thresh',stddev_thresh)
    
  def set_calibrate(self):
    # set calibration flag
    self.add_var_opt('calibrate', '') # no variable required
    
  def set_verbose(self):
    # set verbose flag
    self.add_var_opt('verbose', '') # no variable required
    
  def set_bininput(self):
    # set binary input file flag
    self.add_var_opt('binary-input', '') # no variable required
    
  def set_binoutput(self):
    # set binary output file flag
    self.add_var_opt('binary-output', '') # no variable required
    
  def set_response_function(self,response_function):
    # set reponse function file
    self.add_var_opt('response-file',response_function)
    
  def set_coefficient_file(self,coefficient_file):
    # set the file containing the calibration coefficients (e.g alpha and gammas)
    self.add_var_opt('coefficient-file',coefficient_file)
    
  def set_sensing_function(self,sensing_function):
    # set file containing the sensing function for calibration
    self.add_var_opt('sensing-function',sensing_function)
    
  def set_open_loop_gain(self,open_loop_gain):
    # set file containing the open loop gain for calibration
    self.add_var_opt('open-loop-gain',open_loop_gain)

  def set_scale_fac(self,scale_fac):
    # set scale factor for calibrated data
    self.add_var_opt('scale-factor',scale_fac)

  def set_high_pass(self,high_pass):
    # set high-pass frequency for calibrated data
    self.add_var_opt('high-pass-freq',high_pass)

      
"""
  Pulsar parameter estimation pipeline utilities
"""
class ppeJob(pipeline.CondorDAGJob, pipeline.AnalysisJob):
  """
  A parameter estimation job
  """
  def __init__(self,execu,univ,logpath):
    self.__executable = execu # set executable
    self.__universe = univ # set condor universe
    pipeline.CondorDAGJob.__init__(self, self.__universe, self.__executable)
    pipeline.AnalysisJob.__init__(self, None)
    
    self.add_condor_cmd('getenv','True')
    
    # set log files for job
    self.set_stdout_file(logpath+'/ppe-$(cluster).out')
    self.set_stderr_file(logpath+'/ppe-$(cluster).err')
    self.set_sub_file('ppe.sub')

class ppeNode(pipeline.CondorDAGNode, pipeline.AnalysisNode):
  """
  A pes runs an instance of the parameter estimation code in a condor DAG.
  """
  def __init__(self,job):
    """
    job = A CondorDAGJob that can run an instance of parameter estimation code.
    """
    pipeline.CondorDAGNode.__init__(self,job)
    pipeline.AnalysisNode.__init__(self)
    
    # initilise job variables
    self.__detectors = None
    self.__par_file = None
    self.__cor_file = None
    self.__input_files = None
    self.__downsample_factor = None
    self.__outfile = None
    self.__outXML = None
    self.__chunk_min = None
    self.__chunk_max = None
    self.__psi_bins = None
    self.__time_bins = None
    self.__prior_file = None
    self.__ephem_earth = None
    self.__ephem_sun = None
    self.__model_type = None
    
    self.__Nlive = None
    self.__Nmcmc = None
    self.__Nruns = None
    self.__tolerance = None
    self.__randomseed = None
    
    self.__covariance = None
    self.__temperature = None
    self.__kDTree = None
    self.__kDNCell = None
    self.__kDUpdateFactor = None
    self.__diffev = None
    
    self.__inject_file = None
    self.__inject_output = None
    self.__fake_data = None
    self.__fake_psd = None
    self.__fake_starts = None
    self.__fake_lengths = None
    self.__fake_dt = None
    self.__scale_snr = None
    
    self.__sample_files = None
    self.__sample_nlives = None
    self.__prior_cell = None
    
    self.__mismatch = None
    self.__mm_factor = None
    
    self.__oldChunks = None
    
    self.__verbose = False
    
  def set_detectors(self,detectors):
    # set detectors
    self.add_var_opt('detectors',detectors)
    self.__detectors = detectors
  
  def set_verbose(self):
    # set to run code in verbose mode
    self.add_var_opt('verbose', '')
    self.__verbose = True
  
  def set_par_file(self,parfile):
    # set pulsar parameter file
    self.add_var_opt('par-file',parfile)
    self.__par_file = parfile
    
  def set_cor_file(self,corfile):
    # set pulsar parameter correlation matrix file
    self.add_var_opt('cor-file',corfile)
    self.__cor_file = corfile
    
  def set_input_files(self, inputfiles):
    # set data files for analysing
    self.add_var_opt('input-files', inputfiles)
    self.__input_files = inputfiles
    
  def set_downsample_factor(self, ds):
    # set downsampling factor for input files
    self.add_var_opt('downsample-factor', ds)
    self.__downsample_factor = ds
  
  def set_outfile(self, of):
    # set the output file
    self.add_var_opt('outfile', of)
    self.__outfile = of
    
  def set_outXML(self, ox):
    # set the output XML file
    self.add_var_opt('outXML',ox)
    self.__outXML = ox
    
  def set_chunk_min(self, cmin):
    # set the minimum chunk length
    self.add_var_opt('chunk-min',cmin)
    self.__chunk_min = cmin
    
  def set_chunk_max(self, cmax):
    # set the maximum chunk length
    self.add_var_opt('chunk-max', cmax)
    self.__chunk_max = cmax
    
  def set_psi_bins(self, pb):
    # set the number of bins in psi for the psi vs time lookup table
    self.add_var_opt('psi-bins', pb)
    self.__psi_bins = pb

  def set_time_bins(self, tb):
    # set the number of bins in time for the psi vs time lookup table
    self.add_var_opt('time-bins',tb)
    self.__time_bins = tb
    
  def set_prior_file(self,pf):
    # set the prior ranges file
    self.add_var_opt('prior-file',pf)
    self.__prior_file = pf
    
  def set_ephem_earth(self,ee):
    # set earth ephemeris file
    self.add_var_opt('ephem-earth',ee)
    self.__ephem_earth = ee
    
  def set_ephem_sun(self,es):
    # set sun ephemeris file
    self.add_var_opt('ephem-sun',es)
    self.__ephem_sun = es
    
  def set_model_type(self,mt):
    # set the pulsar model type
    self.add_var_opt('model-type',mt)
    self.__model_type = mt
    
  def set_Nlive(self,nl):
    # set number of live points
    self.add_var_opt('Nlive',nl)
    self.__Nlive = nl
    
  def set_Nmcmc(self,nm):
    # set number of MCMC iterations
    self.add_var_opt('Nmcmc',nm)
    self.__Nmcmc = nm
    
  def set_Nruns(self,nr):
    # set number of internal nested sample runs
    self.add_var_opt('Nruns',nr)
    self.__Nruns = nr
    
  def set_tolerance(self, tol):
    # set tolerance criterion for finishing nested sampling
    self.add_var_opt('tolerance', tol)
    self.__tolerance = tol
    
  def set_randomseed(self, rs):
    # set random number generator seed
    self.add_var_opt('randomseed', rs)
    self.__randomseed = rs
    
  def set_covariance(self, cov):
    # set the fraction of time to use covariance of live points as proposal
    self.add_var_opt('covariance', cov)
    self.__covariance = cov
    
  def set_temperature(self, temp):
    # set temperature scale for covariance proposal
    self.add_var_opt('temperature', temp)
    self.__temperature = temp
    
  def set_kDTree(self,kdt):
    # set fraction of time to use k-d tree of live points as proposal
    self.add_var_opt('kDTree',kdt)
    self.__kDTree = kdt
    
  def set_kDNCell(self,kdc):
    # set number of live points in a k-d tree cell for the proposal
    self.add_var_opt('kDNCell',kdc)
    self.__kDNCell = kdc
    
  def set_kDUpdateFactor(self,kdf):
    # set how often the k-d tree gets updated
    self.add_var_opt('kDUpdateFactor',kdf)
    self.__kDUpdateFactor = kdf
    
  def set_diffev(self,de):
    # set fraction of time to use differential evolution as proposal
    self.add_var_opt('diffev',de)
    self.__diffev = de

  def set_inject_file(self,ifil):
    # set a pulsar parameter file from which to make an injection
    self.add_var_opt('inject-file',ifil)
    self.__inject_file = ifil

  def set_inject_output(self,iout):
    # set filename in which to output the injected signal
    self.add_var_opt('inject-output',iout)
    self.__inject_output = iout
    
  def set_fake_data(self,fd):
    # set the detectors from which to generate fake data
    self.add_var_opt('fake-data',fd)
    self.__fake_data = fd

  def set_fake_psd(self,fp):
    # set the PSDs of the fake data
    self.add_var_opt('fake-psd',fp)
    self.__fake_psd = fp
    
  def set_fake_starts(self,fs):
    # set the start times of the fake data
    self.add_var_opt('fake-starts',fs)
    self.__fake_starts = fs

  def set_fake_lengths(self,fl):
    # set the lengths of the fake data
    self.add_var_opt('fake-lengths',fl)
    self.__fake_lengths = fl
  
  def set_fake_dt(self,fdt):
    # set the sample interval of the fake data
    self.add_var_opt('fake-dt',fdt)
    self.__fake_dt = fdt
    
  def set_scale_snr(self,ssnr):
    # set the SNR of the injected signal
    self.add_var_opt('scale-snr',ssnr)
    self.__scale_snr = ssnr
    
  def set_scale_snr(self,ssnr):
    # set the SNR of the injected signal
    self.add_var_opt('scale-snr',ssnr)
    self.__scale_snr = ssnr
    
  def set_sample_files(self,ssf):
    # set the nested sample files to be used as a prior
    self.add_var_opt('sample-files',ssf)
    self.__sample_files = ssf
    
  def set_sample_nlives(self,snl):
    # set the number of live points for the nested sample files
    self.add_var_opt('sample-nlives',snl)
    self.__sample_nlives = snl
    
  def set_prior_cell(self,pc):
    # set the k-d tree cell size for the prior
    self.add_var_opt('prior-cell',pc)
    self.__prior_cell = pc
    
  def set_mismatch(self,mm):
    # set maximum phase mismatch at which to recalculate the phase model
    self.add_var_opt('mismatch',mm)
    self.__mismatch = mm
    
  def set_mm_factor(self,mmf):
    # set the downsampling factor of the phase model when calculating mismatch
    self.add_var_opt('mm-factor',mmf)
    self.__mm_factor = mmf
  
  def set_OldChunks(self):
    # use the old data segmentation routine i.e. 30 min segments
    self.add_var_opt('oldChunks', '')
    self.__oldChunks = True