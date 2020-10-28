import pandas as pd
import numpy as np
import datetime
import time
from datetime import date
import random
import collections
import matplotlib.dates as mdates
import matplotlib.pyplot as plt
import matplotlib
from p_reporting import m_montecarlo as mmo                          # from reporting module drawdown functions


def statis(my_original, my_guarantee, my_time):
    init_capital = my_guarantee
    print(f'Initial capital = {init_capital}')
    final_cum_cap = my_original.sum() + my_guarantee
    print(f'Final cumulated capital = {final_cum_cap}')
    final_net_cap = my_original.sum()
    print(f'Final net capital = {final_net_cap}')
    min_capital = my_original.cumsum().min() + my_guarantee
    print(f'Minimum capital = {min_capital}')
    max_capital = my_original.cumsum().max() + my_guarantee
    print(f'Maximum capital = {max_capital}')

    net_profit_prc = (my_original.sum() + my_guarantee) / my_guarantee
    print(f'Total Net profit percentage = {net_profit_prc}')
    annual_profit_prc = ((my_original.sum() + my_guarantee) / my_guarantee) ** (1 / my_time) - 1
    print(f'Annual Net profit percentage = {annual_profit_prc}')

    max_drawdown, max_drawdown_pct = mmo.max_dd(my_guarantee, my_original)
    print(f'Maximum drawdown = {max_drawdown}')
    print(f'Maximum drawdown in percentage = {max_drawdown_pct}')

    sharpe = my_original.cumsum().diff().mean() / my_original.cumsum().diff().std() * np.sqrt(252)
    print(f'Annual sharpe = {sharpe}')
    recovery_factor = my_original.sum() / max_drawdown
    print(f'Recovery factor = {recovery_factor}')
    profit_factor = (0 - 1) * np.nansum(np.where(my_original >= 0, my_original, np.nan)) / \
                    np.nansum(np.where(my_original < 0, my_original, np.nan))
    print(f'Profit factor = {profit_factor}')

    winners_pct = my_original[my_original >= 0].count()/my_original.count()
    print(f'Percentage of winners = {winners_pct}')
    losers_pct = my_original[my_original < 0].count()/my_original.count()
    print(f'Percentage of losers = {losers_pct}')
    win_avg = my_original[my_original >= 0].sum()/my_original[my_original >= 0].count()
    print(f'Average profit  = {win_avg}')
    lost_avg = my_original[my_original < 0].sum()/my_original[my_original < 0].count()
    print(f'Average lost  = {lost_avg}')
    lowest_cap = np.min(my_original.sum()+my_guarantee)
    print(f'Lowest Capital  = {lowest_cap}')

# Maximum negatives traders in a row
    trades = my_original.tolist()
    neg_count = 0
    max_neg_count = 0
    for n in range(len(trades)):
        if trades[n] < 0:
            neg_count += 1
        else:
            neg_count = 0
        if neg_count > max_neg_count:
            max_neg_count = neg_count

    print(f'Maximum negatives trades in a row = {max_neg_count}')


def monthly_table(my_original, my_date_serie, my_guarantee):             # new dataframe for monthly results
    df_sry = pd.DataFrame((my_original.cumsum() + my_guarantee).tolist(), index=my_date_serie, columns=['capital'])
    df_sry_res = df_sry.resample('M').last().pct_change()

    my_years = []
    for i in df_sry_res.index:                                                # years for index
        my_year = i.year
        if my_year not in my_years:
            my_years.append(my_year)

    df_sry_tab = pd.DataFrame(0.0, columns=[1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 'total'], index=my_years)
    cont = 0
    for i in df_sry_res.index:                                                  # information in the table
        df_sry_tab[i.month][i.year] = df_sry_res['capital'][i]
        if cont == 0:
            a = df_sry.resample('M').last()
            df_sry_tab[i.month][i.year] = (a['capital'].iloc[0] - my_guarantee) / my_guarantee
        cont = cont + 1

    b = df_sry.resample('A').last()
    c = df_sry.resample('A').last().pct_change()
    for i in range(len(df_sry_tab.index)):                           # totalizing figures
        j = c.index.year[i]
        if i == 0:
            df_sry_tab['total'][j] = (b.iloc[i] - 24000) / 24000
        else:
            df_sry_tab['total'][j] = c.iloc[i]

    return df_sry_tab


def plot_cap_evo(my_original, my_date_serie, my_guarantee):                # capital evolution plot
    my_plot_1 = pd.DataFrame((my_original.cumsum() + my_guarantee).tolist(), columns=['original'],
                             index=my_date_serie.tolist())
    fig, ax = plt.subplots(figsize=(12, 8))
    ax.plot(my_plot_1['original'], lw=3, color="b", alpha=.8, label="Original")
    ax.fill_between(my_date_serie.tolist(), my_plot_1['original'], my_guarantee, facecolor='green',
                    where=my_plot_1['original'] >= my_guarantee)
    ax.fill_between(my_date_serie.tolist(), my_plot_1['original'], my_guarantee, facecolor='red',
                    where=my_plot_1['original'] < my_guarantee)
    ax.axhline(my_guarantee, color="black")
    ax.legend()
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m'))
    ax.grid(True)
    ax.set_title('Capital Evolution', fontweight="bold")
    plt.ylabel("Results in $")
    plt.xlabel('Dates')
    plt.show()
    plt.close()


def plot_mc_simul(my_df_mc, my_original, my_date_serie, my_guarantee):         # monte carlo simulation plot
    my_plot_0 = pd.DataFrame((my_original.cumsum() + my_guarantee).tolist(), columns=['original'],
                             index=my_date_serie.tolist())
    
    #my_plot_0 = my_df_mc
    #my_idx = pd.to_datetime(my_date_serie)
    #my_plot_0 = pd.DataFrame(my_df_mc.iloc[:, 0:1000].to_numpy(), index=my_idx)
    #print(f'My type data-serie {type(my_idx)}, My dataframe info() {my_plot_0.info()}')
    #my_plot_0.set_index(my_idx, inplace=True)

    fig, ax = plt.subplots(figsize=(12, 8))
    #ax.plot(my_plot_0.cumsum() + my_guarantee, lw=1, alpha=.8)
    ax.plot(my_original.cumsum() + my_guarantee, lw=3, color="b", alpha=.8, label="Original")
    ax.axhline(my_guarantee, color="black")
    ax.legend()
    ax.set_title('Monte Carlo Simulation', fontweight="bold")
    plt.ylabel("Results")
    plt.xlabel("Dates")
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m'))
    plt.show()
    plt.close()


def plot_minmax_cap(my_df_mc, my_original, my_date_serie, my_guarantee):        # min and max in monte carlo simulation
    # Step 1 = localize the min and max in dataframe
    lst = my_df_mc.shape[0] - 1
    val_max = 0.0
    val_min = 999999.9
    my_idx_min = 0
    my_idx_max = 0
    for h in range(len(my_df_mc.columns) - 1):
        val = my_df_mc[h].cumsum()[lst]
        if val > val_max:
            val_max = val
            my_idx_max = h
        if val < val_min:
            val_min = val
            my_idx_min = h

    # step 2 we generate the plot
    my_idx = pd.to_datetime(my_date_serie)
    print(f'My type data-serie {type(my_idx)}')
    my_plot_8 = pd.DataFrame((my_original.cumsum()+my_guarantee).tolist(), columns=['original'],
                             index=my_idx)
    my_plot_8[my_idx_min] = (my_df_mc[my_idx_min].cumsum()+my_guarantee).tolist()
    my_plot_8[my_idx_max] = (my_df_mc[my_idx_max].cumsum()+my_guarantee).tolist()

    fig, ax = plt.subplots(figsize=(12,8))
    ax.plot(my_original, lw=2, color="b", alpha=.8, label="Original")
    ax.plot(my_plot_8[my_idx_max], lw=2, color="g", alpha=.8, label="Max")
    ax.plot(my_plot_8[my_idx_min], lw=2, color="r", alpha=.8, label="Min")
    ax.axhline(my_guarantee, color="black")
    ax.legend()
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m'))
    ax.grid(True)
    ax.set_title('Capital evolution', fontweight="bold")
    plt.ylabel("Results in $")
    plt.xlabel('Dates')
    plt.show()
    plt.close()


def plot_drawdown_amt(my_original, my_date_serie, my_guarantee):   # drawdown evolution plot in amount
    # step 1 calculate de drawdown evolution in a dataframe
    my_dd = mmo.max_dd_evol(my_guarantee, my_original)

    # step 2 generate the dataframe and plot
    my_plot_2 = pd.DataFrame((my_dd['dd'] * (0 - 1)).tolist(), columns=['drawdown'],
                             index=my_date_serie.tolist())
    my_plot_2['max_drawdown'] = (my_dd['max_dd'] * (0 - 1)).tolist()

    fig, ax = plt.subplots(figsize=(12, 8))
    ax.plot(my_plot_2['drawdown'], lw=1, color="black", alpha=.8, label="drawdown")
    ax.plot(my_plot_2['max_drawdown'], lw=3, color="red", alpha=.8, label="max-drawdown")
    ax.fill_between(my_date_serie.tolist(), my_plot_2['drawdown'], 0, facecolor='yellow')
    ax.axhline(0, color="black")
    ax.legend()
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m'))
    ax.grid(True)
    ax.set_title('Drawdown evolution', fontweight="bold")
    plt.ylabel("Drawdown in $")
    plt.xlabel('Dates')
    plt.show()
    plt.close()


def plot_drawdown_pct(my_original, my_date_serie, my_guarantee):   # drawdown evolution plot in percentage
    # step 1 calculate de drawdown evolution in a dataframe
    my_dd = mmo.max_dd_evol(my_guarantee, my_original)

    # step 2 generate the dataframe and plot
    my_plot_3 = pd.DataFrame((my_dd['pct_dd'] * (0 - 100)).tolist(), columns=['drawdown'],
                             index=my_date_serie.tolist())
    my_plot_3['max_drawdown'] = (my_dd['pct_max_dd'] * (0 - 100)).tolist()
    fig, ax = plt.subplots(figsize=(12, 8))
    ax.plot(my_plot_3['drawdown'], lw=1, color="black", alpha=.8, label="drawdown")
    ax.plot(my_plot_3['max_drawdown'], lw=3, color="red", alpha=.8, label="max-drawdown")
    ax.fill_between(my_date_serie.tolist(), my_plot_3['drawdown'], 0, facecolor='yellow')
    ax.axhline(0, color="black")
    ax.legend()
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m'))
    ax.grid(True)
    ax.set_title('Drawdown evolution', fontweight="bold")
    plt.ylabel("Drawdown in %")
    plt.xlabel('Ocurrences')
    plt.show()
    plt.close()


def plot_netpft_percentil(my_df_quart):
    my_plot_4 = pd.DataFrame(my_df_quart['netprofit'].tolist(), columns=['netprofit'])
    fig, ax = plt.subplots(figsize=(12, 8))
    ax.plot(my_plot_4['netprofit'], lw=3, color="b", alpha=.8, label="Net Profit")
    ax.axhline(0, color="black")
    ax.legend()
    ax.grid(True)
    ax.set_title('Net Profit / Percentils', fontweight="bold")
    plt.ylabel("Net Profit in $")
    plt.xlabel('Percentils')
    plt.show()
    plt.close()


def plot_annual_netpft_pct_percentil(my_df_quart):
    my_plot_5 = pd.DataFrame((my_df_quart['annual_netprofit_prc'] * 100).tolist(), columns=['netprofit'])
    fig, ax = plt.subplots(figsize=(12, 8))
    ax.plot(my_plot_5['netprofit'], lw=3, color="b", alpha=.8, label="Annual Net Profit %")
    ax.axhline(0, color="black")
    ax.legend()
    ax.grid(True)
    ax.set_title('Annual Net Profit Percentage / Percentils', fontweight="bold")
    plt.ylabel("Annual Net Profit %")
    plt.xlabel('Percentils')
    plt.show()
    plt.close()

def plot_max_drawdown_pct_percentil(my_df_quart):
    my_plot_6 = pd.DataFrame((my_df_quart['MDD_prc'] * 100).tolist(), columns=['MDD'])
    fig, ax = plt.subplots(figsize=(12, 8))
    ax.plot(my_plot_6['MDD'], lw=3, color="b", alpha=.8, label="Max Drawdown %")
    ax.axhline(0, color="black")
    ax.legend()
    ax.grid(True)
    ax.set_title('Maximum Drawdown Percentage / Percentils', fontweight="bold")
    plt.ylabel("Max Drawdown %")
    plt.xlabel('Percentils')
    plt.show()
    plt.close()

def plot_profit_distribution(my_original, my_guarantee):
    fig, ax = plt.subplots()
    data = (my_original.cumsum() + my_guarantee).pct_change().tolist()

    num, bins, patches = ax.hist(data, edgecolor='white', linewidth=1,
                               bins=[-0.10, -0.05, 0.0, 0.05, 0.10, 0.15, 0.2, 0.25, 0.3])
    for i in range(8):
        if (num[i] > 0) & (bins[i] >= 0) & (bins[i+1] >= 0):
            patches[i].set_facecolor('green')
        elif num[i] > 0:
            patches[i].set_facecolor('green')

