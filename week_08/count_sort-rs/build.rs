extern crate cc;

fn main() {
  cc::Build::new()
    .file("../people.c")
    .include("../people.h")
    .compile("people");
}