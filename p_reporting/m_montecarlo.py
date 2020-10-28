import pandas as pd
import numpy as np
import datetime
import time
from datetime import date
import random
import collections
import matplotlib.pyplot as plt
import matplotlib


def monte_carlo_num(my_df, my_guarantee, my_time):
    # Random estimation for 1000 capital curves in Montecarlo simulation
    # step 1
    mc = np.random.choice(my_df[(my_df['result-amt'] != 0) & (my_df['result-accum-amt'] != 0)]['result-amt'],
                          size=(my_df[(my_df['result-amt'] != 0) & (my_df['result-accum-amt'] != 0)]['result-amt']
                                .count(), 1000), replace=True)
    df_mc = pd.DataFrame(mc)
    df_mc['original'] = my_df[(my_df['result-amt'] != 0) & (my_df['result-accum-amt'] != 0)]['result-amt'].tolist()

    # Statistics around montecarlo simulation
    # step 2 statistics of montecarlo

    pd.options.mode.chained_assignment = None         # default='warn' This is to avoid warnings of chained-assignment
    guarantee = my_guarantee                             # two times the broker guarantee (5-10% del valor subyacente)
    times = my_time                                           # tiempo en años

    feature_list = ['random', 'netprofit', 'netprofit_prc', 'annual_netprofit_prc', 'MDD', 'MDD_prc', 'exposure',
                    'ANP-MDD', 'sharpe', 'recoveryfactor', 'profitfactor', 'winners_prc', 'avg_profit',
                    'losers_prc', 'avg_lost', 'lowest_cap']
    df_stat = pd.DataFrame(0.0, index=np.arange(1001), columns=feature_list)

    for w in range(1001):
        jaj = w
        if w == 1000:
            jaj = 'original'
        df_stat['random'][w] = jaj
        df_stat['netprofit'][w] = df_mc[jaj].sum()
        df_stat['netprofit_prc'][w] = (df_mc[jaj].sum() + guarantee) / guarantee
        if df_stat['netprofit_prc'][w] > 0:
            df_stat['annual_netprofit_prc'][w] = ((df_mc[jaj].sum() + guarantee) / guarantee) ** (1 / times) - 1
        else:
            df_stat['annual_netprofit_prc'][w] = 0.0
        df_stat['MDD'][w], df_stat['MDD_prc'][w] = max_dd(guarantee, df_mc[jaj])
        df_stat['exposure'][w] = 38 / 88
        if df_stat['netprofit_prc'][w] > 0:
            df_stat['ANP-MDD'][w] = df_stat['annual_netprofit_prc'][w] / df_stat['MDD'][w]
        else:
            df_stat['ANP-MDD'][w] = 0.0
        df_stat['sharpe'][w] = df_mc[jaj].cumsum().diff().mean() / df_mc[jaj].cumsum().diff().std() * np.sqrt(252)
        df_stat['recoveryfactor'][w] = df_stat['netprofit'][w] / df_stat['MDD'][w]
        df_stat['profitfactor'][w] = (0 - 1) * np.nansum(np.where(df_mc[jaj] >= 0, df_mc[jaj], np.nan)) / np.nansum(
            np.where(df_mc[jaj] < 0, df_mc[jaj], np.nan))
        df_stat['winners_prc'][w] = df_mc[df_mc[jaj] >= 0][jaj].count() / df_mc[jaj].count()
        df_stat['avg_profit'][w] = np.nanmean(np.where(df_mc[jaj] >= 0, df_mc[jaj], np.nan))
        df_stat['losers_prc'][w] = df_mc[df_mc[jaj] < 0][jaj].count() / df_mc[jaj].count()
        df_stat['avg_lost'][w] = np.nanmean(np.where(df_mc[jaj] < 0, df_mc[jaj], np.nan))
        df_stat['lowest_cap'] = np.min(df_mc[jaj].sum()+guarantee)

    # step 3 Quartiles dataframe

    df_quart = pd.DataFrame(0.0, columns=range(15), index=range(101))
    for k in range(1, 15):
        df_quart[k] = np.percentile(df_stat[df_stat.columns[k]].tolist(), range(101))
    df_quart.columns = feature_list

    return df_mc, df_stat, df_quart

# Function Max Drawdown evolution


def max_dd_evol(guarantee, my_serie):                    # to get a dataframe with drawdown evolution
    original = my_serie.tolist()
    l = len(original)

    cm_list = []
    dd_list = []
    dd_mx_list = []
    dd_pc_list = []
    dd_pc_mx_list = []
    my_cum = guarantee
    my_cum_max = 0.0
    my_dd = 0.0
    my_dd_max = 0.0
    my_dd_pc = 0.0
    my_dd_pc_max = 0.0
    for e in range(l):
        my_cum = my_cum + original[e]                                       # cumulator
        if my_cum > my_cum_max:                                             # if increase cumulator dd=0
            my_cum_max = my_cum                                             # new max cumulator
            my_dd = 0
        else:
            my_dd = my_cum_max - my_cum                               # dd = max cumulator - current cumulator
        if my_dd > my_dd_max:                                         # if max dd
            my_dd_max = my_dd
        if my_cum_max > 0:
            my_dd_pc = my_dd / my_cum_max                             # dd percentage
        else:
            my_dd_pc = 0.0
        if my_dd_pc > my_dd_pc_max:                                   # if max dd_pc
            my_dd_pc_max = my_dd_pc
        cm_list.append(my_cum)                                          # list for cumulator
        dd_list.append(my_dd)                                           # list for dd
        dd_mx_list.append(my_dd_max)                                    # list for dd_max
        dd_pc_list.append(my_dd_pc)                                     # list for dd_pc
        dd_pc_mx_list.append(my_dd_pc_max)                              # list for dd_pc_max

    my_dd = pd.DataFrame(0.0, columns=['original', 'cum', 'dd', 'max_dd', 'pct_dd', 'pct_max_dd'], index=range(l))
    my_dd['original'] = my_serie
    my_dd['cum'] = cm_list
    my_dd['dd'] = dd_list
    my_dd['max_dd'] = dd_mx_list
    my_dd['pct_dd'] = dd_pc_list
    my_dd['pct_max_dd'] = dd_pc_mx_list

    return my_dd

# Function Max Drawdown evolution


def max_dd(guarantee, my_serie):                             # to get the max drawdown in total and percentage
    original = my_serie.tolist()
    l = len(original)

    cm_list = []
    dd_list = []
    dd_mx_list = []
    dd_pc_list = []
    dd_pc_mx_list = []
    my_cum = guarantee
    my_cum_max = 0.0
    my_dd = 0.0
    my_dd_max = 0.0
    my_dd_pc = 0.0
    my_dd_pc_max = 0.0
    for e in range(l):
        my_cum = my_cum + original[e]                                   # cumulator
        if my_cum > my_cum_max:                                         # if increase cumulator dd=0
            my_cum_max = my_cum                                         # new max cumulator
            my_dd = 0
        else:
            my_dd = my_cum_max - my_cum                                     # dd = max cumulator - current cumulator
        if my_dd > my_dd_max:                                               # if max dd
            my_dd_max = my_dd
        if my_cum_max > 0:
            my_dd_pc = my_dd / my_cum_max                                   # dd percentage
        else:
            my_dd_pc = 0.0
        if my_dd_pc > my_dd_pc_max:                                         # if max dd_pc
            my_dd_pc_max = my_dd_pc
        cm_list.append(my_cum)                                          # list for cumulator
        dd_list.append(my_dd)                                           # list for dd
        dd_mx_list.append(my_dd_max)                                    # list for dd_max
        dd_pc_list.append(my_dd_pc)                                     # list for dd_pc
        dd_pc_mx_list.append(my_dd_pc_max)                              # list for dd_pc_max

    my_max_dd = max(dd_list)
    my_max_pct_dd = max(dd_pc_list)

    return my_max_dd, my_max_pct_dd
