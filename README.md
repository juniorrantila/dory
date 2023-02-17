# The Dory webserver

**Dory** is a small webserver

## Build instructions

This project is only tested to compile with the `clang` compiler on
linux. You will need to install it along with `meson` and `ninja`.
For faster rebuilds you may install `ccache` as well.

### Setup:

```sh
meson build
```

### Build:

```sh
ninja -C build
```

After building, the Dory executable will be found in
`./build/src/`.

When developing you may want to run the following command:

```sh
. meta/environment.sh
```

This will add `/path/to/src` to your `PATH`, meaning you
will be able write `dory` instead of `./build/src/dory`
to start the webserver.

## Usage

```sh
dory -p <port>
```
