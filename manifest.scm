(use-modules
 (srfi srfi-1)
 (guix profiles)
 (gnu packages cross-base))

(define binutils-arm-none-eabi
  (cross-binutils "arm-none-eabi"))

(define the-manifest
  (concatenate-manifests
   (list
    (packages->manifest
     (list
      binutils-arm-none-eabi))
    (specifications->manifest
     (list
      "bash"
      "cmake"
      "coreutils"
      "diffutils"
      "gdb-arm-none-eabi"
      "git"
      "grep"
      "make"
      "ninja"
      "nss-certs"
      "qemu"
      "sed")))))

the-manifest
