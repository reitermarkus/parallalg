use std::fs::File;
use std::io::{Read, BufReader};
use std::time::Duration;

extern crate ocl;
use ocl::{ProQue, Device, DeviceType, Platform, Event};
use ocl::core::{enqueue_kernel, get_event_profiling_info, ProfilingInfo, QUEUE_PROFILING_ENABLE};
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

  let kernel = pro_que.kernel_builder("reduce")
                      .global_work_offset([0])
                      .local_work_size([local_work_size])
                      .global_work_size([global_work_size])
                      .arg_named("bytes", Some(&bytes_buffer))
                      .arg_local::<usize>(local_work_size)
                      .arg_named("length", &n)
                      .arg_named("result", Some(&result_buffer))
                      .build()?;

  let mut event = Event::empty();

  let ones: u64 = benchmark!("OpenCL (total)", {
    let mut result = vec![0; groups];

    unsafe {
      enqueue_kernel(
        &pro_que.queue(), &kernel,
        pro_que.dims().dim_count(),
        Some(kernel.default_global_work_offset().to_lens().unwrap()),
        &kernel.default_global_work_size().to_lens().unwrap(),
        Some(kernel.default_local_work_size().to_lens().unwrap()),
        None::<Event>, Some(&mut event)
      )?;
    }

    result_buffer.read(&mut result).enq()?;

    let start = get_event_profiling_info(&event, ProfilingInfo::Start).unwrap().time().unwrap();
    let end = get_event_profiling_info(&event, ProfilingInfo::End).unwrap().time().unwrap();

    let (ones, time) = benchmark! {
      result.par_iter().sum()
    };

    print_benchmark!("OpenCL (on-device)", Duration::new(0, (end - start) as u32) + time);

    ones
  });

  println!("{}", ones);

  Ok(())
}

fn main() {
  reduce().unwrap()
}
