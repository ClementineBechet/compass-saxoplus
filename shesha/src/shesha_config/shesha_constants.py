#!/usr/local/bin/python2.7
# encoding: utf-8
'''
Created on 5 juil. 2017

@author: vdeo

Numerical constants for shesha
Config enumerations for safe-typing

'''

import numpy as np

RAD2ARCSEC = 3600. * 360. / (2 * np.pi)
ARCSEC2RAD = 2. * np.pi / (360. * 3600.)
RAD2DEG = 180. / np.pi
DEG2RAD = np.pi / 180.


def check_enum(cls, name):
    '''
        Create a safe-type enum instance from bytes contents
    '''

    if not bytes(name.encode('UTF-8')) in cls.__dir__.values():
        raise ValueError(
                "Invalid enumeration value for enum %s, value %s" %
                (cls, name))
    return name


class DmType:
    '''
        Types of deformable mirrors
    '''

    PZT = b'pzt'
    TT = b'tt'
    KL = b'kl'


class PatternType:
    '''
        Types of Piezo DM patterns
    '''

    SQUARE = b'square'
    HEXA = b'hexa'
    HEXAM4 = b'hexaM4'


class KLType:
    '''
        Possible KLs for computations
    '''

    KOLMO = b'kolmo'
    KARMAN = b'karman'


class InfluType:
    '''
        Influence function types
    '''

    DEFAULT = b'default'
    RADIALSCHWARTZ = b'radialSchwartz'
    SQUARESCHWARTZ = b'squareSchwartz'
    BLACKNUTT = b'blacknutt'
    GAUSSIAN = b'gaussian'
    BESSEL = b'bessel'


class ControllerType:
    '''
        Controller types
    '''

    GENERIC = b'generic'
    LS = b'ls'
    MV = b'mv'
    CURED = b'cured'
    GEO = b'geo'
    KALMAN_C = b'kalman_CPU'
    KALMAN_G = b'kalman_GPU'
    KALMAN_UN = b'kalman_uninitialized'


class WFSType:
    '''
        WFS Types
    '''

    GEO = b'geo'
    SH = b'sh'
    PYR = b'pyr'
    PYRHR = b'pyrhr'
    ROOF = b'roof'


class TargetImageType:
    '''
        Target Images
    '''

    SE = b'se'
    LE = b'le'


class ApertureType:
    '''
        Telescope apertures
    '''
    GENERIC = b'Generic'
    EELT_NOMINAL = b'EELT-Nominal'
    EELT_BP1 = b'EELT-BP1'
    EELT_BP3 = b'EELT-BP3'
    EELT_BP5 = b'EELT-BP5'
    EELT_CUSTOM = b'EELT-Custom'
    VLT = b'VLT'


class SpiderType:
    '''
        Spiders
    '''
    FOUR = b'four'
    SIX = b'six'