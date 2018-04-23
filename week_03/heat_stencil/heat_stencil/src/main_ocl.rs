extern crate ocl;
use ocl::{Buffer, ProQue, Device, DeviceType, Platform};

use std::mem;

mod print_temperature;
use print_temperature::print_temperature;

#[macro_use]
mod benchmark;

fn temp() -> ocl::Result<()> {
  let kernel_source = r#"
    __kernel void calc_temp(__global float const* matrix_a, __global float* matrix_b, ulong n, ulong source_x, ulong source_y) {
      size_t i = get_global_id(0);
      size_t j = get_global_id(1);

      // Center stays constant (the heat is still on).
      if (i == source_x && j == source_y) {
        matrix_b[i * n + j] = matrix_a[i * n + j];
      } else {
        // Get current temperature at (i,j).
        float tc = matrix_a[i * n + j];

        // Get temperatures left/right and up/down.
        float tl = j !=  0     ? matrix_a[i * n + (j - 1)] : tc;
        float tr = j != n - 1  ? matrix_a[i * n + (j + 1)] : tc;
        float tu = i !=  0     ? matrix_a[(i - 1) * n + j] : tc;
        float td = i != n - 1  ? matrix_a[(i + 1) * n + j] : tc;

        // Update temperature at current point.
        matrix_b[i * n + j] = tc + 0.2 * (tl + tr + tu + td + (-4.0 * tc));
      }
    }
  "#;

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

  let kernel = pro_que.kernel_builder("calc_temp")
                      .global_work_offset([0, 0])
                      .global_work_size([n, n])
                      .arg_named("input", None::<&Buffer<f32>>)
                      .arg_named("output", None::<&Buffer<f32>>)
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
