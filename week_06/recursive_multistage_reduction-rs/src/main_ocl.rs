use std::fs::File;
use std::io::{Read, BufReader};

extern crate ocl;
use ocl::{ProQue, Device, DeviceType, Platform};
use ocl::enums::DeviceInfo;
use ocl::enums::DeviceInfoResult::MaxWorkGroupSize;

extern crate rand;

extern crate rayon;
use rayon::prelude::*;

mod random_bytes;
use random_bytes::random_bytes;

#[macro_use]
mod benchmark;

macro_rules! multiple {
  ($v:expr, $d:expr) => {{
    let rest = $v % $d;
    if rest == 0 { $v } else { $v + ($d - rest) }
  }}
}

fn reduce() -> ocl::Result<()> {
  let n = 1_000_000;
  let seed = [0; 32];

  let bytes = random_bytes(n, seed);

  let file = File::open("../kernel.cl")?;
  let mut buf_reader = BufReader::new(file);
  let mut kernel_source = String::new();
  buf_reader.read_to_string(&mut kernel_source)?;

  let device_list = Device::list(Platform::default(), Some(DeviceType::GPU))?;
  let device = device_list.last().expect("No GPU found.");

  let pro_que = ProQue::builder().src(kernel_source)
                                 .dims(n)
                                 .device(device)
                                 .build()?;

  let bytes_buffer = pro_que.buffer_builder().copy_host_slice(&bytes).build()?;
  let result_buffer = pro_que.create_buffer::<u64>()?;

  let local_work_size = {
    if let Ok(MaxWorkGroupSize(size)) = device.info(DeviceInfo::MaxWorkGroupSize) {
      size
    } else {
      1
    }
  };

  let global_work_size = multiple!(n, local_work_size);

  let groups = global_work_size / local_work_size;

  let ones: u64 = benchmark! {
    bytes.par_iter().sum()
  };
  println!("{}", ones);

  let kernel = pro_que.kernel_builder("reduce")
                      .global_work_offset([0])
                      .local_work_size([local_work_size])
                      .global_work_size([global_work_size])
                      .arg_named("bytes", Some(&bytes_buffer))
                      .arg_local::<usize>(local_work_size)
                      .arg_named("length", &n)
                      .arg_named("result", Some(&result_buffer))
                      .build()?;



  let ones: u64 = benchmark! {
    let mut result = vec![0; groups];
    unsafe { kernel.enq()?; }
    result_buffer.read(&mut result).enq()?;
    result.iter().sum()
  };

  println!("{}", ones);

  Ok(())
}

fn main() {
  reduce().unwrap()
}
