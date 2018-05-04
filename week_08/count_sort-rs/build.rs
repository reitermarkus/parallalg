extern crate cc;

fn main() {
  cc::Build::new()
    .file("../count_sort/people.c")
    .include("../count_sort/people.h")
    .compile("people");
}