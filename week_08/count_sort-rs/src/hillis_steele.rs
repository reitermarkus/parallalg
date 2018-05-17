use std::fs::File;
use std::io::{Read, BufReader};
use std::mem;
use std::time::Duration;

extern crate ocl;
use ocl::{Buffer, ProQue, Device, DeviceType, Platform, Event};
use ocl::flags::{QUEUE_PROFILING_ENABLE};
use ocl::enums::{DeviceInfo, DeviceInfoResult::MaxWorkGroupSize, ProfilingInfo::Start, ProfilingInfo::End};

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

fn hillis_steele() -> ocl::Result<()> {
  let n = 4_000;
  let seed = [0; 32];

  let bytes = random_bytes(n, seed);

  let file = File::open("../hillissteele.cl")?;
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

  let kernel = pro_que.kernel_builder("hillis_steele")
                      .global_work_offset([0])
                      .local_work_size([local_work_size])
                      .arg_named("bytes", None::<&Buffer<u64>>)
                      .arg_local::<usize>(n * 2)
                      .arg_named("result", None::<&Buffer<u64>>)
                      .arg_named("length", n)
                      .build()?;

  let ones: u64 = benchmark!("OpenCL (total)", {
    let mut length = n;
    let mut global_work_size;

    global_work_size = multiple!(length, local_work_size);

    kernel.set_arg("bytes", Some(&bytes_buffer))?;
    kernel.set_arg("length", length)?;
    kernel.set_arg("result", Some(&result_buffer))?;

    let mut event = Event::empty();

    unsafe {
    kernel.cmd()
        .global_work_size([global_work_size])
        .enew(&mut event)
        .enq()?;
    }

    kernel.default_queue().unwrap().finish()?;

    let start = event.profiling_info(Start)?.time().unwrap();
    let end = event.profiling_info(End)?.time().unwrap();

    let duration = Duration::new(0, (end - start) as u32);

    mem::swap(&mut bytes_buffer, &mut result_buffer);

    let mut result = vec![0; length];

    bytes_buffer.read(&mut result).enq()?;

    print_benchmark!("OpenCL (on-device)", duration);

    result[length - 1]
  });

  println!("{}", ones);

  Ok(())
}

fn main() {
  hillis_steele().unwrap()
}
