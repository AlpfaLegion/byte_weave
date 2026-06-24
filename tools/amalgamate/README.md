# ByteWeave Amalgamate Tool

`amalgamate.py` builds the single-header distribution of ByteWeave.

The tool packages the library headers and implementation into a single
self-contained header:

```text
<target_dir>/byte_weave/byte_weave.hpp
```

This generated header can then be used as the header-only distribution of the
library by adding `<target_dir>` to the include path and including:

```cpp
#include <byte_weave/byte_weave.hpp>
```

## Usage

```sh
python tools/amalgamate/amalgamate.py \
  --source_dir include/byte_weave \
  --target_dir single_include
```

Options:

- `-s, --source_dir` - source directory with ByteWeave header files.
- `-t, --target_dir` - target include directory where the combined header will
  be written.
- `-v, --verbose` - print additional information during generation.
- `-f, --force_save` - overwrite the target header if it already exists.

## Example

From the repository root:

```sh
python tools/amalgamate/amalgamate.py \
  -s include/byte_weave \
  -t single_include \
  -v \
  -f
```

This command regenerates:

```text
single_include/byte_weave/byte_weave.hpp
```
