<h1 data-importer="text" align="center">FileCrypt</h1>

###

<p data-importer="text" align="left">
A simple command-line file encryption tool written in C++ using libsodium.
</p>

###

<h2 data-importer="text" align="center">Features</h2>

###

<p data-importer="text" align="left">
1. Password-based file encryption<br>
2. Uses Argon2id for key derivation<br>
3. Uses XSalsa20-Poly1305 authenticated encryption (crypto_secretbox)<br>
4. Random salt generated for every file<br>
5. Random nonce generated for every encryption operation<br>
6. Detects incorrect passwords and file tampering<br>
7. Cross-platform (Linux, macOS, Windows)
</p>

###

<h2 data-importer="text" align="center">Cryptography</h2>

###

<p data-importer="text" align="left">
FileCrypt uses the following cryptographic primitives provided by libsodium:
<br><br>
1. Key Derivation: Argon2id (crypto_pwhash)<br>
2. Encryption: XSalsa20<br>
3. Authentication: Poly1305<br>
4. Authenticated Encryption API: crypto_secretbox
</p>

###

<h2 data-importer="text" align="center">Usage</h2>

###

<p data-importer="text" align="left">
Encrypt a file:
</p>

```bash
./filecrypt encrypt secret.txt secret.enc
