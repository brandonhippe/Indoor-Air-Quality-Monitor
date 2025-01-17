#! /usr/bin/env python
""" crypto.cipher.rijndael_test

    Tests for the rijndael encryption algorithm

    Copyright (c) 2002 by Paul A. Lambert
    Read LICENSE.txt for license information.
"""
from crypto.cipher.rijndael import Rijndael
from crypto.cipher.base     import noPadding
from binascii               import a2b_hex
import unittest

class Rijndael_TestVectors(unittest.TestCase):
    """ Test Rijndael algorithm using know values."""

    def testGladman_dev_vec(self):
        """ All 25 combinations of block and key size.
            These test vectors were generated by Dr Brian Gladman
            using the program aes_vec.cpp  <brg@gladman.uk.net> 24th May 2001.
            vectors in file: dev_vec.txt
            http://fp.gladman.plus.com/cryptography_technology/rijndael/index.htm
        """
        def RijndaelTestVec(i, key, pt, ct):
            """ Run single AES test vector with any legal blockSize
                and any legal key size. """
            bkey, plainText, cipherText = a2b_hex(key), a2b_hex(pt), a2b_hex(ct)
            kSize = len(bkey)
            bSize = len(cipherText) # set block size to length of block
            alg = Rijndael(bkey, keySize=kSize, blockSize=bSize, padding=noPadding())

            self.assertEqual( alg.encrypt(plainText),  cipherText )
            self.assertEqual( alg.decrypt(cipherText), plainText )

        RijndaelTestVec( i   = 'dev_vec.txt 16 byte block, 16 byte key',
                         key = '2b7e151628aed2a6abf7158809cf4f3c',
                         pt  = '3243f6a8885a308d313198a2e0370734',
                         ct  = '3925841d02dc09fbdc118597196a0b32')

        RijndaelTestVec( i   = 'dev_vec.txt 16 byte block, 20 byte key',
                         key = '2b7e151628aed2a6abf7158809cf4f3c762e7160',
                         pt  = '3243f6a8885a308d313198a2e0370734',
                         ct  = '231d844639b31b412211cfe93712b880')

        RijndaelTestVec( i   = 'dev_vec.txt 16 byte block, 24 byte key',
                         key = '2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da5',
                         pt  = '3243f6a8885a308d313198a2e0370734',
                         ct  = 'f9fb29aefc384a250340d833b87ebc00')

        RijndaelTestVec( i   = 'dev_vec.txt 16 byte block, 28 byte key',
                         key = '2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da56a784d90',
                         pt  = '3243f6a8885a308d313198a2e0370734',
                         ct  = '8faa8fe4dee9eb17caa4797502fc9d3f')

        RijndaelTestVec( i   = 'dev_vec.txt 16 byte block, 32 byte key',
                         key = '2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da56a784d9045190cfe',
                         pt  = '3243f6a8885a308d313198a2e0370734',
                         ct  = '1a6e6c2c662e7da6501ffb62bc9e93f3')

        RijndaelTestVec( i   = 'dev_vec.txt 20 byte block, 16 byte key',
                         key = '2b7e151628aed2a6abf7158809cf4f3c',
                         pt  = '3243f6a8885a308d313198a2e03707344a409382',
                         ct  = '16e73aec921314c29df905432bc8968ab64b1f51')

        RijndaelTestVec( i   = 'dev_vec.txt 20 byte block, 20 byte key',
                         key = '2b7e151628aed2a6abf7158809cf4f3c762e7160',
                         pt  = '3243f6a8885a308d313198a2e03707344a409382',
                         ct  = '0553eb691670dd8a5a5b5addf1aa7450f7a0e587')

        RijndaelTestVec( i   = 'dev_vec.txt 20 byte block, 24 byte key',
                         key = '2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da5',
                         pt  = '3243f6a8885a308d313198a2e03707344a409382',
                         ct  = '73cd6f3423036790463aa9e19cfcde894ea16623')

        RijndaelTestVec( i   = 'dev_vec.txt 20 byte block, 28 byte key',
                         key = '2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da56a784d90',
                         pt  = '3243f6a8885a308d313198a2e03707344a409382',
                         ct  = '601b5dcd1cf4ece954c740445340bf0afdc048df')

        RijndaelTestVec( i   = 'dev_vec.txt 20 byte block, 32 byte key',
                         key = '2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da56a784d9045190cfe',
                         pt  = '3243f6a8885a308d313198a2e03707344a409382',
                         ct  = '579e930b36c1529aa3e86628bacfe146942882cf')

        RijndaelTestVec( i   = 'dev_vec.txt 24 byte block, 16 byte key',
                         key = '2b7e151628aed2a6abf7158809cf4f3c',
                         pt  = '3243f6a8885a308d313198a2e03707344a4093822299f31d',
                         ct  = 'b24d275489e82bb8f7375e0d5fcdb1f481757c538b65148a')

        RijndaelTestVec( i   = 'dev_vec.txt 24 byte block, 20 byte key',
                         key = '2b7e151628aed2a6abf7158809cf4f3c762e7160',
                         pt  = '3243f6a8885a308d313198a2e03707344a4093822299f31d',
                         ct  = '738dae25620d3d3beff4a037a04290d73eb33521a63ea568')

        RijndaelTestVec( i   = 'dev_vec.txt 24 byte block, 24 byte key',
                         key = '2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da5',
                         pt  = '3243f6a8885a308d313198a2e03707344a4093822299f31d',
                         ct  = '725ae43b5f3161de806a7c93e0bca93c967ec1ae1b71e1cf')

        RijndaelTestVec( i   = 'dev_vec.txt 24 byte block, 28 byte key',
                         key = '2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da56a784d90',
                         pt  = '3243f6a8885a308d313198a2e03707344a4093822299f31d',
                         ct  = 'bbfc14180afbf6a36382a061843f0b63e769acdc98769130')

        RijndaelTestVec( i   = 'dev_vec.txt 24 byte block, 32 byte key',
                         key = '2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da56a784d9045190cfe',
                         pt  = '3243f6a8885a308d313198a2e03707344a4093822299f31d',
                         ct  = '0ebacf199e3315c2e34b24fcc7c46ef4388aa475d66c194c')

        RijndaelTestVec( i   = 'dev_vec.txt 28 byte block, 16 byte key',
                         key = '2b7e151628aed2a6abf7158809cf4f3c',
                         pt  = '3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa9',
                         ct  = 'b0a8f78f6b3c66213f792ffd2a61631f79331407a5e5c8d3793aceb1')

        RijndaelTestVec( i   = 'dev_vec.txt 28 byte block, 20 byte key',
                         key = '2b7e151628aed2a6abf7158809cf4f3c762e7160',
                         pt  = '3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa9',
                         ct  = '08b99944edfce33a2acb131183ab0168446b2d15e958480010f545e3')

        RijndaelTestVec( i   = 'dev_vec.txt 28 byte block, 24 byte key',
                         key = '2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da5',
                         pt  = '3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa9',
                         ct  = 'be4c597d8f7efe22a2f7e5b1938e2564d452a5bfe72399c7af1101e2')

        RijndaelTestVec( i   = 'dev_vec.txt 28 byte block, 28 byte key',
                         key = '2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da56a784d90',
                         pt  = '3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa9',
                         ct  = 'ef529598ecbce297811b49bbed2c33bbe1241d6e1a833dbe119569e8')

        RijndaelTestVec( i   = 'dev_vec.txt 28 byte block, 32 byte key',
                         key = '2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da56a784d9045190cfe',
                         pt  = '3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa9',
                         ct  = '02fafc200176ed05deb8edb82a3555b0b10d47a388dfd59cab2f6c11')

        RijndaelTestVec( i   = 'dev_vec.txt 32 byte block, 16 byte key',
                         key = '2b7e151628aed2a6abf7158809cf4f3c',
                         pt  = '3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa98ec4e6c8',
                         ct  = '7d15479076b69a46ffb3b3beae97ad8313f622f67fedb487de9f06b9ed9c8f19')

        RijndaelTestVec( i   = 'dev_vec.txt 32 byte block, 20 byte key',
                         key = '2b7e151628aed2a6abf7158809cf4f3c762e7160',
                         pt  = '3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa98ec4e6c8',
                         ct  = '514f93fb296b5ad16aa7df8b577abcbd484decacccc7fb1f18dc567309ceeffd')

        RijndaelTestVec( i   = 'dev_vec.txt 32 byte block, 24 byte key',
                         key = '2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da5',
                         pt  = '3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa98ec4e6c8',
                         ct  = '5d7101727bb25781bf6715b0e6955282b9610e23a43c2eb062699f0ebf5887b2')

        RijndaelTestVec( i   = 'dev_vec.txt 32 byte block, 28 byte key',
                         key = '2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da56a784d90',
                         pt  = '3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa98ec4e6c8',
                         ct  = 'd56c5a63627432579e1dd308b2c8f157b40a4bfb56fea1377b25d3ed3d6dbf80')

        RijndaelTestVec( i   = 'dev_vec.txt 32 byte block, 32 byte key',
                         key = '2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da56a784d9045190cfe',
                         pt  = '3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa98ec4e6c8',
                         ct  = 'a49406115dfb30a40418aafa4869b7c6a886ff31602a7dd19c889dc64f7e4e7a')

# Make this test module runnable from the command prompt
if __name__ == "__main__":
    unittest.main()


