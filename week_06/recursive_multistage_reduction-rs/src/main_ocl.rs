use std::mem;
use std::fs::File;
use std::io::{Read, BufReader};

extern crate ocl;
use ocl::{ProQue, Device, DeviceType, Platform};
use ocl::enums::DeviceInfo;
use ocl::enums::DeviceInfoResult::MaxWorkGroupSize;

extern crate rayon;
use rayon::prelude::*;

extern crate rand;

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
  let n = 16;
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

  let mut bytes_buffer = pro_que.buffer_builder().copy_host_slice(&bytes).build()?;
  let mut result_buffer = pro_que.create_buffer::<u64>()?;

  let local_work_size = {
    if let Ok(MaxWorkGroupSize(size)) = device.info(DeviceInfo::MaxWorkGroupSize) {
      size
    } else {
      1
    }
  };

  let kernel = pro_que.kernel_builder("reduce")
                      .global_work_offset([0])
                      .local_work_size([local_work_size])
                      .global_work_size([multiple!(n / 2, local_work_size)])
                      .arg_named("bytes", Some(&bytes_buffer))
                      .arg_local::<usize>(n / 2)
                      .arg_named("length", &n)
                      .arg_named("result", Some(&result_buffer))
                      .build()?;

  unsafe { kernel.enq()?; }

  let mut result = vec![0; n / 2];
  result_buffer.read(&mut result).enq()?;

  let ones: u64 = bytes.par_iter().sum();
  println!("{}", ones);

  println!("{:?}", bytes);
  println!("{:?}", result);

  Ok(())
}

fn main() {
  reduce().unwrap()
}
