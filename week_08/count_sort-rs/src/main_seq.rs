extern crate libc;
use libc::c_char;
use std::ffi::CStr;

extern {
  fn gen_name() -> *const c_char;
}

fn main() {
  let c_buf = unsafe { gen_name() };
  let c_str = unsafe { CStr::from_ptr(c_buf) };
  let str_slice = c_str.to_str().unwrap();
}