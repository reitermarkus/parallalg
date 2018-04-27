use std::fs::File;
use std::io::{Read, BufReader};
use std::mem;
use std::time::Duration;

extern crate ocl;
use ocl::{ProQue, Device, DeviceType, Platform, Event};
use ocl::flags::{QUEUE_PROFILING_ENABLE};
use ocl::enums::{DeviceInfo, DeviceInfoResult::MaxWorkGroupSize, ProfilingInfo::Start, ProfilingInfo::End};

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

  let ones: u64 = benchmark!("Serial", {
    bytes.iter().sum()
  });
  println!("{}", ones);

  let ones: u64 = benchmark!("Parallel", {
    bytes.par_iter().sum()
  });
  println!("{}", ones);

  let file = File::open("../kernel.cl")?;
  let mut buf_reader = BufReader::new(file);
  let mut kernel_source = String::new();
  buf_reader.read_to_string(&mut kernel_source)?;

  let device_list = Device::list(Platform::default(), Some(DeviceType::GPU))?;
  let device = device_list.last().expect("No GPU found.");

  let pro_que = ProQue::builder().src(kernel_source)
                                 .dims(n)
                                 .device(device)
                                 .queue_properties(QUEUE_PROFILING_ENABLE)
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

  let mut global_work_size = multiple!(n, local_work_size);

  let kernel = pro_que.kernel_builder("reduce")
                      .global_work_offset([0])
                      .local_work_size([local_work_size])
                      .global_work_size([global_work_size])
                      .arg_named("bytes", Some(&bytes_buffer))
                      .arg_local::<usize>(local_work_size)
                      .arg_named("length", &n)
                      .arg_named("result", Some(&result_buffer))
                      .build()?;

  let ones: u64 = benchmark!("OpenCL (total)", {
    let mut length = n;
    let mut result_array_size;

    let mut duration = Duration::new(0, 0);

    loop {
      result_array_size = global_work_size / local_work_size;

      kernel.set_arg("bytes", Some(&bytes_buffer))?;
      //kernel.set_arg(1, local_work_size)?;
      kernel.set_arg("length", &length)?;
      kernel.set_arg("result", Some(&result_buffer))?;

      let mut event = Event::empty();

      unsafe {
        kernel.cmd().enew(&mut event).enq()?;
      }

      kernel.default_queue().unwrap().finish()?;

      let start = event.profiling_info(Start)?.time().unwrap();
      let end = event.profiling_info(End)?.time().unwrap();

      duration += Duration::new(0, (end - start) as u32);

      mem::swap(&mut bytes_buffer, &mut result_buffer);

      if result_array_size == 1 {
        break;
      }

      length = result_array_size;
      global_work_size = multiple!(result_array_size, local_work_size);
    }

    let mut result = vec![0; 1];

    bytes_buffer.read(&mut result).enq()?;

    print_benchmark!("OpenCL (on-device)", duration);

    result[0]
  });

  println!("{}", ones);

  Ok(())
}

fn main() {
  reduce().unwrap()
}
