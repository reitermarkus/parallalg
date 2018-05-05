extern crate cc;

fn main() {
  cc::Build::new()
    .file("../count_sort/tokenize.c")
    .include("../count_sort/tokenize.h")
    .file("../count_sort/people.c")
    .include("../count_sort/people.h")
    .compile("people");
}
