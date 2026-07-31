/** @file
  Root include file for mini Mde Package Base type modules 

  This is the include file for any module of type base. Base modules only use
  types defined via this include file and can be ported easily to any
  environment. There are a set of base libraries in the Mde Package that can
  be used to implement base modules.

Copyright (c) 2026, Henry Shin <henry.shin@thundersoft.com>. All rights reserved.<BR>
Portions Copyright (c) 2006 - 2021, Intel Corporation. All rights reserved.<BR>
Portions copyright (c) 2008 - 2009, Apple Inc. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent


**/
#pragma once

#define  BIT0   0x00000001
#define  BIT1   0x00000002
#define  BIT2   0x00000004
#define  BIT3   0x00000008
#define  BIT4   0x00000010
#define  BIT5   0x00000020
#define  BIT6   0x00000040
#define  BIT7   0x00000080
#define  BIT8   0x00000100
#define  BIT9   0x00000200
#define  BIT10  0x00000400
#define  BIT11  0x00000800
#define  BIT12  0x00001000
#define  BIT13  0x00002000
#define  BIT14  0x00004000
#define  BIT15  0x00008000
#define  BIT16  0x00010000
#define  BIT17  0x00020000
#define  BIT18  0x00040000
#define  BIT19  0x00080000
#define  BIT20  0x00100000
#define  BIT21  0x00200000
#define  BIT22  0x00400000
#define  BIT23  0x00800000
#define  BIT24  0x01000000
#define  BIT25  0x02000000
#define  BIT26  0x04000000
#define  BIT27  0x08000000
#define  BIT28  0x10000000
#define  BIT29  0x20000000
#define  BIT30  0x40000000
#define  BIT31  0x80000000
#define  BIT32  0x0000000100000000ULL
#define  BIT33  0x0000000200000000ULL
#define  BIT34  0x0000000400000000ULL
#define  BIT35  0x0000000800000000ULL
#define  BIT36  0x0000001000000000ULL
#define  BIT37  0x0000002000000000ULL
#define  BIT38  0x0000004000000000ULL
#define  BIT39  0x0000008000000000ULL
#define  BIT40  0x0000010000000000ULL
#define  BIT41  0x0000020000000000ULL
#define  BIT42  0x0000040000000000ULL
#define  BIT43  0x0000080000000000ULL
#define  BIT44  0x0000100000000000ULL
#define  BIT45  0x0000200000000000ULL
#define  BIT46  0x0000400000000000ULL
#define  BIT47  0x0000800000000000ULL
#define  BIT48  0x0001000000000000ULL
#define  BIT49  0x0002000000000000ULL
#define  BIT50  0x0004000000000000ULL
#define  BIT51  0x0008000000000000ULL
#define  BIT52  0x0010000000000000ULL
#define  BIT53  0x0020000000000000ULL
#define  BIT54  0x0040000000000000ULL
#define  BIT55  0x0080000000000000ULL
#define  BIT56  0x0100000000000000ULL
#define  BIT57  0x0200000000000000ULL
#define  BIT58  0x0400000000000000ULL
#define  BIT59  0x0800000000000000ULL
#define  BIT60  0x1000000000000000ULL
#define  BIT61  0x2000000000000000ULL
#define  BIT62  0x4000000000000000ULL
#define  BIT63  0x8000000000000000ULL

/**
  Returns a 16-bit signature built from 2 ASCII characters.

  This macro returns a 16-bit value built from the two ASCII characters specified
  by A and B.

  @param  A    The first ASCII character.
  @param  B    The second ASCII character.

  @return A 16-bit value built from the two ASCII characters specified by A and B.

**/
#define SIGNATURE_16(A, B)  ((A) | (B << 8))

/**
  Returns a 32-bit signature built from 4 ASCII characters.

  This macro returns a 32-bit value built from the four ASCII characters specified
  by A, B, C, and D.

  @param  A    The first ASCII character.
  @param  B    The second ASCII character.
  @param  C    The third ASCII character.
  @param  D    The fourth ASCII character.

  @return A 32-bit value built from the two ASCII characters specified by A, B,
          C and D.

**/
#define SIGNATURE_32(A, B, C, D)  (SIGNATURE_16 (A, B) | (SIGNATURE_16 (C, D) << 16))

/**
  Returns a 64-bit signature built from 8 ASCII characters.

  This macro returns a 64-bit value built from the eight ASCII characters specified
  by A, B, C, D, E, F, G,and H.

  @param  A    The first ASCII character.
  @param  B    The second ASCII character.
  @param  C    The third ASCII character.
  @param  D    The fourth ASCII character.
  @param  E    The fifth ASCII character.
  @param  F    The sixth ASCII character.
  @param  G    The seventh ASCII character.
  @param  H    The eighth ASCII character.

  @return A 64-bit value built from the two ASCII characters specified by A, B,
          C, D, E, F, G and H.

**/
#define SIGNATURE_64(A, B, C, D, E, F, G, H) \
    (SIGNATURE_32 (A, B, C, D) | ((UINT64) (SIGNATURE_32 (E, F, G, H)) << 32))
