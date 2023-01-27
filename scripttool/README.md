# Master Sword Script Tool
A tool designed to handle decrypting, unpacking, packing and encrypting script packages for the Master Sword mod. While it uses the same logic as the legacy code, it was entirely rewritten on C++17 with "modern" designs. Using it should be much simpler than the manual option.

## Usage
### Decrypting
```
scripttool [--ignore-checksum] -decrypt decrypted_file encrypted_file
```

Decrypts the `encrypted_file` into `decrypted_file`. Optionally can be told to ignore the checksum if manual changes to the decrypted file were made.

### Encrypting
```
scripttool -encrypt encrypted_file decrypted_file
```

Encrypts the `decrypted_file` into `encrypted_file`.

### Unpacking
```
scripttool -encrypt output_dir decrypted_file
```

Unpacks all files in `decrypted_file` into `output_dir`.

### Packaging
```
scripttool -encrypt decrypted_file input_dir
```

Packages all files in `input_dir` into the `decrypted_file`.

## Special Thanks
- Thanks go out to [spark](https://www.msremake.com/members/spark.3993/) from the MS:Continued forum for the pointers.
- Additional thanks go to Aze, for reminding me that this mod even exists.
