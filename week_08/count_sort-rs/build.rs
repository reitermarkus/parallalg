extern crate cc;

fn main() {
  cc::Build::new()
    .file("../people.c")
    .compile("people");
}