@test "example" {
  bin/compile examples/example.code example.asm > /dev/null
  nasm -f elf64 example.asm -o example.o
  gcc -no-pie -o example example.o lib/libclosure.a lib/libstandard.a; 
  run ./example
  [ "$status" -eq 0 ]
  [ "$output" = "2" ]
}

@test "example_lambda" {
  bin/compile examples/example_lambda.code example_lambda.asm > /dev/null
  nasm -f elf64 example_lambda.asm -o example_lambda.o
  gcc -no-pie -o example_lambda example_lambda.o lib/libclosure.a lib/libstandard.a; 
  run ./example_lambda
  [ "$status" -eq 0 ]
  [ "$output" = "2" ]
}

@test "example_lets" {
  bin/compile examples/example_lets.code example_lets.asm > /dev/null
  nasm -f elf64 example_lets.asm -o example_lets.o
  gcc -no-pie -o example_lets example_lets.o lib/libclosure.a lib/libstandard.a; 
  run ./example_lets
  [ "$status" -eq 0 ]
  [ "$output" = "4" ]
}

@test "example_letrec" {
  bin/compile examples/example_letrec.code example_letrec.asm > /dev/null
  nasm -f elf64 example_letrec.asm -o example_letrec.o
  gcc -no-pie -o example_letrec example_letrec.o lib/libclosure.a lib/libstandard.a; 
  run ./example_letrec
  [ "$status" -eq 0 ]
  [ "$output" = "0" ]
}

@test "example_unicode" {
  bin/compile examples/example_unicode.code example_unicode.asm > /dev/null
  nasm -f elf64 example_unicode.asm -o example_unicode.o
  gcc -no-pie -o example_unicode example_unicode.o lib/libclosure.a lib/libstandard.a; 
  run ./example_unicode
  [ "$status" -eq 0 ]
  [ "$output" = "2" ]
}

@test "example_closure" {
  bin/compile examples/example_closure.code example_closure.asm > /dev/null
  nasm -f elf64 example_closure.asm -o example_closure.o
  gcc -no-pie -o example_closure example_closure.o lib/libclosure.a lib/libstandard.a; 
  run ./example_closure
  [ "$status" -eq 0 ]
  [ "$output" = "3" ]
}

@test "example_first_class" {
  bin/compile examples/example_first_class.code example_first_class.asm > /dev/null
  nasm -f elf64 example_first_class.asm -o example_first_class.o
  gcc -no-pie -o example_first_class example_first_class.o lib/libclosure.a lib/libstandard.a; 
  run ./example_first_class
  [ "$status" -eq 0 ]
  [ "$output" = "8" ]
}

@test "example_defs" {
  bin/compile examples/example_defs.code example_defs.asm > /dev/null
  nasm -f elf64 example_defs.asm -o example_defs.o
  gcc -no-pie -o example_defs example_defs.o lib/libclosure.a lib/libstandard.a; 
  run ./example_defs
  [ "$status" -eq 0 ]
  [ "$output" = "8" ]
}

@test "example_defs2" {
  bin/compile examples/example_defs2.code example_defs2.asm > /dev/null
  nasm -f elf64 example_defs2.asm -o example_defs2.o
  gcc -no-pie -o example_defs2 example_defs2.o lib/libclosure.a lib/libstandard.a; 
  run ./example_defs2
  [ "$status" -eq 0 ]
  [ "$output" = "1" ]
}

@test "example_mult" {
  bin/compile examples/example_mult.code example_mult.asm > /dev/null
  nasm -f elf64 example_mult.asm -o example_mult.o
  gcc -no-pie -o example_mult example_mult.o lib/libclosure.a lib/libstandard.a; 
  run ./example_mult
  [ "$status" -eq 0 ]
  [ "$output" = "12" ]
}

@test "example_negative" {
  bin/compile examples/example_negative.code example_negative.asm > /dev/null
  nasm -f elf64 example_negative.asm -o example_negative.o
  gcc -no-pie -o example_negative example_negative.o lib/libclosure.a lib/libstandard.a; 
  run ./example_negative
  [ "$status" -eq 0 ]
  [ "$output" = "-4" ]
}

@test "example_rec" {
  bin/compile examples/example_rec.code example_rec.asm > /dev/null
  nasm -f elf64 example_rec.asm -o example_rec.o
  gcc -no-pie -o example_rec example_rec.o lib/libclosure.a lib/libstandard.a; 
  run ./example_rec
  [ "$status" -eq 0 ]
  [ "$output" = "6" ]
}

@test "example_factorial" {
  bin/compile examples/example_factorial.code example_factorial.asm > /dev/null
  nasm -f elf64 example_factorial.asm -o example_factorial.o
  gcc -no-pie -o example_factorial example_factorial.o lib/libclosure.a lib/libstandard.a; 
  run ./example_factorial
  [ "$status" -eq 0 ]
  [ "$output" = "120" ]
}

@test "example_compose" {
  bin/compile examples/example_compose.code example_compose.asm > /dev/null
  nasm -f elf64 example_compose.asm -o example_compose.o
  gcc -no-pie -o example_compose example_compose.o lib/libclosure.a lib/libstandard.a; 
  run ./example_compose
  [ "$status" -eq 0 ]
  [ "$output" = "4" ]
}

@test "error_unmatched_open" {
  run bin/compile examples/error_unmatched_open.code error_unmatched_open.asm
  [ "$status" -gt 0 ]
}

@test "error_unmatched_close" {
  run bin/compile examples/error_unmatched_close.code error_unmatched_close.asm
  [ "$status" -gt 0 ]
}

@test "error_lambda_short" {
  run bin/compile examples/error_lambda_short.code error_lambda_short.asm
  [ "$status" -gt 0 ]
}
