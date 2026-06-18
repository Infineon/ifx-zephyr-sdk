# Audio Front-End Sample (Module Consumer)

This directory is a sample consumer in `ifx-zephyr-sdk`.

## Intended role

- Demonstrate application-side use of audio front-end functionality.
- Consume AFE Zephyr glue and asset modules via `west`.

## Module ownership

The reusable Zephyr module glue belongs in:

- `c:\downstream\audio-front-end\ifx-zephyr-lib`

The sample repo (`ifx-zephyr-sdk`) is for applications that use modules.

## Expected west topology

- sample repo: `ifx-zephyr-sdk`
- glue module: `modules/lib/ifx-audio-front-end`
- upstream asset: `modules/lib/third_party/audio-front-end`

As migration continues, this sample should avoid owning reusable module glue logic.
