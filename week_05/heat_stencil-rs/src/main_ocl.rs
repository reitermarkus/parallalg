extern crate ocl;
use ocl::{Buffer, ProQue, Device, DeviceType, Platform};
use ocl::enums::DeviceInfo;
use ocl::enums::DeviceInfoResult::MaxWorkGroupSize;

use std::mem;
use std::fs::File;
use std::io::{Read, BufReader};

mod print_temperature;
use print_temperature::print_temperature;

#[macro_use]
mod benchmark;

macro_rules! multiple {
  ($v:expr, $d:expr) => {{
    let rest = $v % $d;
    if rest == 0 { $v } else { $v + ($d - rest) }
  }}
}

fn temp() -> ocl::Result<()> {
  let file = File::open("../heat_stencil/heat_stencil.cl")?;
  let mut buf_reader = BufReader::new(file);
  let mut kernel_source = String::new();
  buf_reader.read_to_string(&mut kernel_source)?;

  let n = 500;

  let device_list = Device::list(Platform::default(), Some(DeviceType::GPU))?;
  let device = device_list.last().expect("No GPU found.");

  let pro_que = ProQue::builder().src(kernel_source)
                                 .dims(n * n)
                                 .device(device)
                                 .build()?;

  let mut matrix_a = vec![273.0; n * n];

  // Add heat source in corner.
  let source_x = n / 4;
  let source_y = n / 4;
  matrix_a[source_x * n + source_y] += 60.0;

  let mut matrix_a_buffer = pro_que.buffer_builder().copy_host_slice(&matrix_a).build()?;
  let mut matrix_b_buffer = pro_que.create_buffer::<f32>()?;

  print_temperature(&matrix_a, n, n);

  let time_steps = n * 100;

  let local_work_size = {
    if let Ok(MaxWorkGroupSize(size)) = device.info(DeviceInfo::MaxWorkGroupSize) {
      [(size as f64).sqrt() as usize, (size as f64).sqrt() as usize]
    } else {
      [1, 1]
    }
  };

  let kernel = pro_que.kernel_builder("calc_temp")
                      .global_work_offset([0, 0])
                      .local_work_size(local_work_size)
                      .global_work_size([multiple!(n, local_work_size[0]), multiple!(n, local_work_size[1])])
                      .arg_named("input", None::<&Buffer<f32>>)
                      .arg_named("output", None::<&Buffer<f32>>)
                      .arg_local::<f32>((local_work_size[0] + 2) * (local_work_size[1] + 2))
                      .arg(&n)
                      .arg(&source_x)
                      .arg(&source_y)
                      .build()?;

  benchmark! {
    for t in 0..time_steps {
      kernel.set_arg("input", Some(&matrix_a_buffer))?;
      kernel.set_arg("output", Some(&matrix_b_buffer))?;

      unsafe { kernel.enq()?; }

      mem::swap(&mut matrix_a_buffer, &mut matrix_b_buffer);

      if t % 1000 == 0 {
        matrix_a_buffer.read(&mut matrix_a).enq()?;
        println!("Step t = {}:", t);
        print_temperature(&matrix_a, n, n);
      }
    }
  };

  Ok(())
}

fn main() {
  if let Err(error) = temp() {
    panic!("{:?}", error);
  }
}
