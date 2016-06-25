{
  packageOverrides = pkgs: {
    opencv3 = pkgs.opencv3.override { enableBloat = true; };
    opencv = pkgs.opencv.override { enableBloat = true; };
  };
}
