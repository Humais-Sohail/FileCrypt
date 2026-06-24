#include <iostream>
#include <sodium.h>
#include <fstream>
#include <string>
#include <vector>

constexpr size_t SALT_SIZE = crypto_pwhash_SALTBYTES;
constexpr size_t NONCE_SIZE = crypto_secretbox_NONCEBYTES;
constexpr size_t KEY_SIZE = crypto_secretbox_KEYBYTES;

std::vector<unsigned char> readFile(const std::string& path)
{
  std::ifstream file(path, std::ios::binary);

  if (!file)
    throw std::runtime_error("Failed to open file");

  file.seekg(0, std::ios::end);
  size_t size = static_cast<size_t>(file.tellg());
  file.seekg(0, std::ios::beg);

  std::vector<unsigned char> data(size);
  file.read(reinterpret_cast<char*>(data.data()), size);

  return data;
}

void writeFile(const std::string& path,
    const std::vector<unsigned char>& data)
{
  std::ofstream file(path, std::ios::binary);

  if (!file)
    throw std::runtime_error("Failed to create file");

  file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

bool deriveKey(const std::string& password,
    const unsigned char salt[SALT_SIZE],
    unsigned char key[KEY_SIZE])
{
  return crypto_pwhash(
      key,
      KEY_SIZE,
      password.c_str(),
      password.size(),
      salt,
      crypto_pwhash_OPSLIMIT_MODERATE,
      crypto_pwhash_MEMLIMIT_MODERATE,
      crypto_pwhash_ALG_DEFAULT) == 0;
}

void encryptFile(const std::string& input,
    const std::string& output,
    const std::string& password)
{
  auto plaintext = readFile(input);

  unsigned char salt[SALT_SIZE];
  unsigned char nonce[NONCE_SIZE];
  unsigned char key[KEY_SIZE];

  randombytes_buf(salt, sizeof(salt));
  randombytes_buf(nonce, sizeof(nonce));

  if (!deriveKey(password, salt, key))
    throw std::runtime_error("Key derivation failed");

  std::vector<unsigned char> ciphertext(
      plaintext.size() + crypto_secretbox_MACBYTES);

  crypto_secretbox_easy(
      ciphertext.data(),
      plaintext.data(),
      plaintext.size(),
      nonce,
      key);

  sodium_memzero(key, sizeof(key));

  std::vector<unsigned char> out;

  out.insert(out.end(), salt, salt + SALT_SIZE);
  out.insert(out.end(), nonce, nonce + NONCE_SIZE);
  out.insert(out.end(), ciphertext.begin(), ciphertext.end());

  writeFile(output, out);

  std::cout << "Encrypted: " << output << '\n';
}

void decryptFile(const std::string& input,
    const std::string& output,
    const std::string& password)
{
  auto fileData = readFile(input);

  if (fileData.size() < SALT_SIZE + NONCE_SIZE +
      crypto_secretbox_MACBYTES)
  {
    throw std::runtime_error("Invalid encrypted file");
  }

  const unsigned char* salt = fileData.data();
  const unsigned char* nonce = fileData.data() + SALT_SIZE;
  const unsigned char* ciphertext =
    fileData.data() + SALT_SIZE + NONCE_SIZE;

  size_t cipherLen =
    fileData.size() - SALT_SIZE - NONCE_SIZE;

  unsigned char key[KEY_SIZE];

  if (!deriveKey(password, salt, key))
    throw std::runtime_error("Key derivation failed");

  std::vector<unsigned char> plaintext(
      cipherLen - crypto_secretbox_MACBYTES);

  if (crypto_secretbox_open_easy(
        plaintext.data(),
        ciphertext,
        cipherLen,
        nonce,
        key) != 0)
  {
    sodium_memzero(key, sizeof(key));
    throw std::runtime_error(
        "Wrong password or corrupted file");
  }

  sodium_memzero(key, sizeof(key));

  writeFile(output, plaintext);

  std::cout << "Decrypted: " << output << '\n';
}

int main(int argc, char* argv[])

{
  std::cout << R"(

  ███████╗██╗██╗     ███████╗ ██████╗██████╗ ██╗   ██╗██████╗ ████████╗
  ██╔════╝██║██║     ██╔════╝██╔════╝██╔══██╗╚██╗ ██╔╝██╔══██╗╚══██╔══╝
  █████╗  ██║██║     █████╗  ██║     ██████╔╝ ╚████╔╝ ██████╔╝   ██║
  ██╔══╝  ██║██║     ██╔══╝  ██║     ██╔══██╗  ╚██╔╝  ██╔═══╝    ██║
  ██║     ██║███████╗███████╗╚██████╗██║  ██║   ██║   ██║        ██║
  ╚═╝     ╚═╝╚══════╝╚══════╝ ╚═════╝╚═╝  ╚═╝   ╚═╝   ╚═╝        ╚═╝

                            .--------.
                           / .------. \
                          / /        \ \
                          | |  ____  | |
                         _| |_/ ___\_| |_
                       .' |_| |     | |_| '.
                       '._____'.___.'_____.'

)";
  std::cout << "\n";
  if (sodium_init() < 0)
  {
    std::cerr << "libsodium init failed\n";
    return 1;
  }

  if (argc != 4)
  {
    std::cout
      << "Usage:\n"
      << "  filecrypt encrypt <input> <output>\n"
      << "  filecrypt decrypt <input> <output>\n";
    return 1;
  }

  std::string mode = argv[1];
  std::string input = argv[2];
  std::string output = argv[3];

  std::string password;

  std::cout << "Password: ";
  std::getline(std::cin, password);

  try
  {
    if (mode == "encrypt")
    {
      encryptFile(input, output, password);
    }
    else if (mode == "decrypt")
    {
      decryptFile(input, output, password);
    }
    else
    {
      std::cerr << "Unknown mode\n";
      return 1;
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "Error: " << e.what() << '\n';
    return 1;
  }

  return 0;
}
