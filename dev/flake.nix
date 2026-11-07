{
  #also generated with claude tools
  inputs = {
    nixpkgs.url = "https://flakehub.com/f/NixOS/nixpkgs/0.1";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, utils }:
    utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        devShells.default = pkgs.mkShell {
          nativeBuildInputs = with pkgs; [
            cmake
            ninja
            pkg-config
            clang-tools # clangd, clang-format
          ];

          buildInputs = with pkgs; [
            sdl3
            sdl3-image
          ];

          # clangd isn't a Nix cc-wrapper, so it doesn't see buildInputs headers
          # automatically. CPATH is read by clang/clangd as extra include dirs.
          # (SDL's headers live in the separate "dev" output.)
          CPATH = pkgs.lib.makeSearchPath "include" [
            pkgs.sdl3.dev
            pkgs.sdl3-image.dev
          ];

          shellHook = ''
            # Let CMake write compile_commands.json for clangd
            export CMAKE_EXPORT_COMPILE_COMMANDS=1
          '';
        };
      }
    );
}

# instructions for full project-aware stuff
#
#nix develop
#cmake -B build -G Ninja -DUSE_SYSTEM_SDL=ON
#ln -sf build/compile_commands.json .
