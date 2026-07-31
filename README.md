# Common PQC Libraries for RISC-V

This repository contains vendor-independent, bare-metal C ports of NIST Post-Quantum Cryptography (PQC) algorithms targeted for RISC-V soft cores. This codebase provides a streamlined environment for running, testing, and validating these cryptographic implementations on RISC-V architectures.

## 🟢 Current Status: Software Ready

All modules have successfully completed initial porting and bare-metal RTL simulation validation.

## Supported Algorithms

| Algorithm | Type | Status | Upstream Source |
|---|---|---|---|
| **ML-KEM** (Kyber) | Key Encapsulation | 🟢 Software Ready | [pq-crystals/kyber](https://github.com/pq-crystals/kyber) |
| **ML-DSA** (Dilithium) | Digital Signature | 🟢 Software Ready | [pq-crystals/dilithium](https://github.com/pq-crystals/dilithium) |
| **SLH-DSA** (Sphincs+) | Digital Signature | 🟢 Software Ready | [sphincs/sphincsplus](https://github.com/sphincs/sphincsplus) |
| **FN-DSA** (Falcon) | Digital Signature | 🟢 Software Ready | [falcon-sign.info](https://falcon-sign.info/) |

## Implementation Notes

*   **Original Codebases:** The core cryptographic implementations retain their original source code and headers. 
*   **Modifications for Bare-Metal:** To facilitate easier RISC-V bare-metal testing and RTL simulation, the Random Number Generator (`rng.c`) has been modified to use hardcoded deterministic values. **Do not use this modified RNG in a production or secure environment.**

---

## License and Acknowledgements

The RISC-V specific wrappers, hardware drivers, ROM generators, and testing environments developed by our project team are distributed under the **MIT License** (see the root `LICENSE` file).

This repository also incorporates several third-party cryptographic reference implementations. We have left all original source code headers and copyright notices intact. These algorithms retain their original, highly permissive open-source licenses:

### Third-Party Licenses

*   **FN-DSA (Falcon):** Provided under the **MIT License**, whose text is included at the start of every source file.
*   **ML-DSA (Dilithium):** Licensed under **Public Domain (CC0)**, **Apache 2.0 License**, or **GPL 2.0 License**. For Keccak and the random number generator, this implementation uses public-domain code from sources and authors listed in the comments at the top of the respective files.
*   **ML-KEM (Kyber):** Licensed under **Public Domain (CC0)** or **Apache 2.0 License**. For Keccak and AES, this implementation uses public-domain code from sources and authors listed in the comments at the top of the respective files.
*   **SLH-DSA (SPHINCS+):** Licensed under **SPDX-License-Identifier: (LicenseRef-SPHINCS-PLUS-Public-Domain OR CC0-1.0 OR 0BSD OR MIT-0) AND MIT**.
