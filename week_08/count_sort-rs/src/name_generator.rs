use libc::{c_char, c_void};

use std::{ffi::CStr};

extern {
  fn gen_name() -> *const c_char;
  fn free_names() -> c_void;
}

static mut NAME_GENERATOR_COUNT: usize = 0;

pub struct NameGenerator {}

impl NameGenerator {
  pub fn new() -> Self {
    unsafe { NAME_GENERATOR_COUNT += 1; }
    NameGenerator {}
  }

  pub fn generate(&self) -> String {
    let c_str = unsafe { CStr::from_ptr(gen_name()) };
    c_str.to_str().unwrap().to_owned()
  }
}

impl Drop for NameGenerator {
  fn drop(&mut self) {
    unsafe {
      NAME_GENERATOR_COUNT -= 1;

      if NAME_GENERATOR_COUNT == 0 {
        free_names();
      }
    }
  }
}
