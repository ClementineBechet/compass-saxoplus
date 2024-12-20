# -*- coding: utf-8 -*-
"""
Created on Tue Jan 19 15:41:57 2016

@author: fferreira
"""
import os
import pandas
import numpy as np
import matplotlib.pyplot as plt
import shesha

SHESHA = os.environ.get('SHESHA_ROOT')
BENCH_SAVEPATH = SHESHA + "data/bench-results/"
plt.ion()


def depouilleSR(filename, version=None):

    store = pandas.HDFStore(filename)
    if(version is None):
        version = shesha.__version__
    try:
        df = store.get(version)
    except KeyError:
        print("No results for svn version : " + version + ", taking " + list(store.keys())[-1] + " version")
        version = list(store.keys())[-1]
        df = store.get(version)

    dates = np.unique(df["date"].values)
    wfs_type = np.unique(df["sensor_type"].values)
    LGS = np.unique(df["LGS"].values)
    noise = np.unique(df["noisy"].values)
    nxsub = np.unique(df["nxsub"].values)
    npix = np.unique(df["npix"].values)
    controllers = np.unique(df["controller"].values)
    centroiders = np.unique(df["centroider"].values)

    for d in dates:
        dd = str(d[2]) + "/" + str(d[1]) + "/" + str(d[0]) + "/"
        for w in wfs_type:
            for lgs_id in LGS:
                if(lgs_id):
                    star = "LGS"
                else:
                    star = "NGS"
                for n in noise:
                    for nx in nxsub:
                        for N in npix:
                            cevalid = []
                            covalid = []
                            plt.figure()
                            plt.title("date:{},{} {}, noise:{},nxsub:{},npix:{}, SR perfs".format(
                                dd, w, star, n, nx, N))
                            plt.ylabel("Strehl ratio")
                            cc = 0
                            colors = ["blue", "red", "yellow",
                                      "orange", "cyan", "purple", "magenta"]
                            width = 0.9 / len(centroiders)
                            for ce in centroiders:
                                ind = 0
                                for co in controllers:
                                    for indx in df.index:
                                        if(df.loc[indx, "date"] == d and df.loc[indx, "sensor_type"] == w
                                           and df.loc[indx, "LGS"] == lgs_id and df.loc[indx, "noisy"] == n
                                           and df.loc[indx, "nxsub"] == nx and df.loc[indx, "npix"] == N
                                           and df.loc[indx, "controller"] == co and df.loc[indx, "centroider"] == ce):
                                            cevalid.append(ce)
                                            covalid.append(co)
                                            plt.bar(int(cc) + ind * (1. / len(
                                                centroiders)), df.loc[indx, "finalSRLE"], width, color=colors[ind], yerr=df.loc[indx, "rmsSRLE"])
                                            plt.text(int(cc) + ind * (1. / len(centroiders)) + width / 4.,
                                                     df.loc[indx, "finalSRLE"] + 0.01, '%d' % int(df.loc[indx, "finalSRLE"] * 100))
                                            plt.text(int(cc) + ind * (1. / len(centroiders)) + width / 4.,
                                                     df.loc[indx, "finalSRLE"] / 2., str(ce + " + " + co), rotation="vertical")
                                            ind += 1
                                            cc += 1. / len(controllers)
                            cevalid = np.unique(cevalid)
                            covalid = np.unique(covalid)
                            if(len(cevalid) == 0):
                                plt.close()
                            else:
                                plt.xticks(
                                    np.arange(len(cevalid)) + 0.5, cevalid)
                                plt.legend(covalid)

    store.close()


def depouillePerf(filename, version=None, mode="profile"):

    store = pandas.HDFStore(filename)
    if(version is None):
        version = shesha.__version__
    try:
        df = store.get(version)
    except KeyError:
        print("No results for git version : " + version + ", taking " + list(store.keys())[-1] + " version")
        version = list(store.keys())[-1]
        df = store.get(version)

    dates = np.unique(df["date"].values)
    wfs_type = np.unique(df["sensor_type"].values)
    LGS = np.unique(df["LGS"].values)
    noise = np.unique(df["noisy"].values)
    nxsub = np.unique(df["nxsub"].values)
    npix = np.unique(df["npix"].values)
    controllers = np.unique(df["controller"].values)
    centroiders = np.unique(df["centroider"].values)

    times = ["move_atmos", "target_trace_atmos", "target_trace_dm", "sensor_trace_atmos",
             "sensor_trace_dm", "comp_img", "docentroids", "docontrol", "applycontrol"]

    for d in dates:
        dd = str(d[2]) + "/" + str(d[1]) + "/" + str(d[0]) + "/"
        for w in wfs_type:
            for lgs_id in LGS:
                if(lgs_id):
                    star = "LGS"
                else:
                    star = "NGS"
                for n in noise:
                    for nx in nxsub:
                        for N in npix:
                            cevalid = []
                            covalid = []
                            pos = []
                            lab = []
                            plt.figure()
                            plt.title("date:{},{} {}, noise:{},nxsub:{},npix:{}, execution profile".format(
                                dd, w, star, n, nx, N))
                            cc = 0
                            colors = ["blue", "green", "red", "yellow",
                                      "orange", "cyan", "purple", "magenta", "darkcyan"]
                            width = 0.9 / len(centroiders)
                            for ce in centroiders:
                                ind = 0
                                for co in controllers:
                                    for indx in df.index:
                                        if(df.loc[indx, "date"] == d and df.loc[indx, "sensor_type"] == w
                                           and df.loc[indx, "LGS"] == lgs_id and df.loc[indx, "noisy"] == n
                                           and df.loc[indx, "nxsub"] == nx and df.loc[indx, "npix"] == N
                                           and df.loc[indx, "controller"] == co and df.loc[indx, "centroider"] == ce):
                                            cevalid.append(ce)
                                            covalid.append(co)
                                            ccc = 0
                                            if(mode == b"full"):
                                                plt.barh(int(
                                                    cc) + ind * (1. / len(centroiders)), df.loc[indx, times[0]], width, color=colors[ccc])
                                                timeb = df.loc[indx, times[0]]
                                                ccc += 1
                                                for i in times[1:]:
                                                    plt.barh(int(
                                                        cc) + ind * (1. / len(centroiders)), df.loc[indx, i], width, color=colors[ccc], left=timeb)
                                                    timeb += df.loc[indx, i]
                                                    ccc += 1
                                            elif(mode == b"profile"):
                                                tottime = 0
                                                for i in times:
                                                    tottime += df.loc[indx, i]
                                                plt.barh(int(cc) + ind * (1. / len(
                                                    centroiders)), df.loc[indx, times[0]] / tottime * 100, width, color=colors[ccc])
                                                timeb = df.loc[indx,
                                                               times[0]] / tottime * 100
                                                ccc += 1
                                                for i in times[1:]:
                                                    plt.barh(int(cc) + ind * (1. / len(
                                                        centroiders)), df.loc[indx, i] / tottime * 100, width, color=colors[ccc], left=timeb)
                                                    if(df.loc[indx, i] / tottime * 100 > 10.):
                                                        plt.text(timeb + df.loc[indx, i] / tottime * 100 / 2, int(cc) + ind * (1. / len(
                                                            centroiders)) + width / 4., '%d' % int(df.loc[indx, i] / tottime * 100) + " %")
                                                    timeb += df.loc[indx,
                                                                    i] / tottime * 100
                                                    ccc += 1
                                            elif(mode == b"framerate"):
                                                plt.barh(int(cc) + ind * (1. / len(centroiders)), 1. /
                                                         df.loc[indx, "iter_time"] * 1000., width, color=colors[ind])
                                                plt.text(1. / df.loc[indx, "iter_time"] * 1000. / 2., int(cc) + ind * (1. / len(
                                                    centroiders)) + width / 4., '%.1f' % float(1. / df.loc[indx, "iter_time"] * 1000.))
                                                ccc += 1

                                            pos.append(
                                                int(cc) + ind * (1. / len(centroiders)) + width / 2.)
                                            lab.append(ce + " + " + co)
                                            ind += 1
                                            cc += 1. / len(controllers)
                            cevalid = np.unique(cevalid)
                            covalid = np.unique(covalid)
                            if(len(cevalid) == 0):
                                plt.close()
                            else:
                                plt.yticks(pos, lab)
                                if(mode == b"full"):
                                    plt.xlabel("Execution time (ms)")
                                    plt.legend(times)
                                elif(mode == b"profile"):
                                    plt.xlabel("Occupation time (%)")
                                    plt.legend(times)
                                elif(mode == b"framerate"):
                                    plt.xlabel("Framerate (frames/s)")

    store.close()
