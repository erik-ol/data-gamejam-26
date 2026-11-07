{
  # generated with claude code
  description = "SDL3 game: native + web (emscripten) builds";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

    # Sources used ONLY for the web build (emscripten has to compile SDL itself).
    # Bump these tags to whatever version you vendored.
    sdl3-src = {
      url = "github:libsdl-org/SDL/release-3.2.20";
      flake = false;
    };
    sdl3-image-src = {
      url = "github:libsdl-org/SDL_image/release-3.2.4";
      flake = false;
    };
  };

  outputs = { self, nixpkgs, sdl3-src, sdl3-image-src }:
    let
      systems = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];
      forAllSystems = f:
        nixpkgs.lib.genAttrs systems (system: f nixpkgs.legacyPackages.${system});
    in
    {
      packages = forAllSystems (pkgs: rec {
        default = native;

        # ---------------------------------------------------------------
        # Native build, using nixpkgs' SDL3 / SDL3_image
        # ---------------------------------------------------------------
        native = pkgs.stdenv.mkDerivation {
          pname = "game";
          version = "0.1.0";
          src = self;

          nativeBuildInputs = with pkgs; [ cmake pkg-config makeWrapper ];
          buildInputs = with pkgs; [ sdl3 sdl3-image ];

          # Tells the (patched) CMakeLists.txt to skip vendored/ and use find_package
          cmakeFlags = [ "-DUSE_SYSTEM_SDL=ON" ];

          installPhase = ''
            runHook preInstall

            mkdir -p $out/libexec/game $out/bin
            install -m755 "$(find . -maxdepth 2 -type f -name main -perm -u+x | head -n1)" \
              $out/libexec/game/main
            cp -r $src/assets $src/data $out/libexec/game/
            chmod -R u+w $out/libexec/game

            # Run from the dir containing assets/ and data/ so relative paths work
            makeWrapper $out/libexec/game/main $out/bin/game \
              --chdir $out/libexec/game

            runHook postInstall
          '';

          meta.mainProgram = "game";
        };

        # ---------------------------------------------------------------
        # Web build (WebAssembly via Emscripten)
        # Output: $out/index.html, main.js, main.wasm, main.data
        # ---------------------------------------------------------------
        web = pkgs.stdenv.mkDerivation {
          pname = "game-web";
          version = "0.1.0";
          src = self;

          nativeBuildInputs = with pkgs; [ cmake emscripten python3 ];

          dontUseCmakeConfigure = true;

          configurePhase = ''
            runHook preConfigure

            # emscripten wants a writable cache + home
            export HOME=$TMPDIR
            export EM_CACHE=$TMPDIR/emscripten-cache
            mkdir -p $EM_CACHE

            # Provide the sources your CMakeLists expects under vendored/
            rm -rf vendored
            mkdir -p vendored
            cp -r ${sdl3-src} vendored/SDL
            cp -r ${sdl3-image-src} vendored/SDL_image
            chmod -R u+w vendored

            emcmake cmake -S . -B build \
              -DCMAKE_BUILD_TYPE=Release \
              -DSDL_SHARED=OFF -DSDL_STATIC=ON \
              -DSDL_TESTS=OFF -DSDL_EXAMPLES=OFF

            runHook postConfigure
          '';

          buildPhase = ''
            runHook preBuild
            cmake --build build -j$NIX_BUILD_CORES
            runHook postBuild
          '';

          installPhase = ''
            runHook preInstall

            mkdir -p $out
            #incredible, thanks claude code
            for f in $(find build -maxdepth 2 -type f -name 'main.*'); do
              case "$f" in
                *.html) cp "$f" $out/index.html ;;
                *)      cp "$f" $out/ ;;
              esac
            done

            runHook postInstall
          '';
        };
      });

      apps = forAllSystems (pkgs: {
        default = {
          type = "app";
          program = "${self.packages.${pkgs.system}.native}/bin/game";
        };

        # nix run .#serve-web  ->  http://localhost:8000
        serve-web = {
          type = "app";
          program = "${pkgs.writeShellScript "serve-web" ''
            echo "Serving on http://localhost:8000"
            exec ${pkgs.python3}/bin/python3 -m http.server 8000 \
              --directory ${self.packages.${pkgs.system}.web}
          ''}";
        };
      });

      devShells = forAllSystems (pkgs: {
        default = pkgs.mkShell {
          nativeBuildInputs = with pkgs; [ cmake pkg-config ninja emscripten ];
          buildInputs = with pkgs; [ sdl3 sdl3-image ];
          # Native dev: cmake -B build -DUSE_SYSTEM_SDL=ON && cmake --build build
        };
      });
    };
}
