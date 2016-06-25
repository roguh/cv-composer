let
    pkgs = import <nixpkgs> { config = import ./opencv_config.nix; };
in rec {
    env = pkgs.stdenv.mkDerivation rec {
        name = "opencv_project";
        src = ./.;
        buildInputs = with pkgs; [
            clang
            cmake
            opencv3
        ];
    };
}
