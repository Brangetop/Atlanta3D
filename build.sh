set -e

cd "$(dirname "$0")/engine"

gcc main.c game_state.c helper.c loaders.c player.c \
    -o atlanta3D_engine \
    -lGL -lGLU -lglut -lm

echo "COMPLETE: ./atlanta3D_engine"
./atlanta3D_engine