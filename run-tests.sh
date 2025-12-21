set -e

dir=$(dirname $0)
pushd $dir

meson setup build -Db_coverage=true -Dtests=enabled --reconfigure
meson compile -C build --clean
meson test -C build || true #--wrapper 'valgrind --leak-check=full'
ninja coverage -C build

