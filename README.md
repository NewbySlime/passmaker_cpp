## Note
I made this in 1 day and I only have 1 years of experience, feel free to critique my spaghetti of a code.

And if 15 chars of password isn't enough for you, just change the ENCRYPTEDSTRMAX macro and compile it.

# passmaker

passmaker is a program to make an encrypted password based on the current domain and your original main password.
This is just a password tool that I make just for fun and the sake of my fascination.
Might not be perfect, use it on your own risk.

## Compilation

Compile the source code with your trusty c++ compiler.

```bash
g++ ./src/pass.cpp
```

## Usage

```bash
./<the_program>.exe

./<the_program>.exe -m<your_main_password_here>
```
"-m" option is used to write/replace your main password (optional, if you already placed your main password).

The resulting password will be put in your clipboard.

## Contributing
For now, I have another projects to do, so this project will be ignored temporarily.

## License
[GPL-3.0](https://choosealicense.com/licenses/gpl-3.0/#)
