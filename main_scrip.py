import argparse
import datetime
import sys
import pandas as pd
import os.path

from p_readingdata import m_acquisition as mac
from p_ranging import m_dataranging as mda               # import from pack ranging the module
from p_operating import m_operation as mop                # import from pack operating the module
from p_analysis import m_longshort as mlo                 # import from pack analysis the module
from p_reporting import m_montecarlo as mmo
from p_reporting import m_statis_plotis as mst


def argument_parser():
    parser = argparse.ArgumentParser(description ='Set arguments path and file input with variable to use')
    parser.add_argument("-p", "--path", help="introduce your path and file input location (csv format with comma"
                                             " delimited)", type=str, dest="path", required=True)
    parser.add_argument("-a", "--algorithm", help="introduce your algorithm to apply in the market)", type=str,
                        dest="algorit", required=False)
    parser.add_argument("-f", "--force", help="write 'force' to force preprocessing all files (existing and not)",
                        type=str, dest="force", required=False)
    args = parser.parse_args()
    return args


def main(path):
    file_input = path
    if not mac.validate_file(file_input):
        sys.exit(f'Error: file/path <{file_input}> does not exist')
    df_input, my_error = mac.data_input(file_input)
    if len(my_error) > 0:
        sys.exit(f'Error: {my_error}>')
    key_config_0_ant = ' '
    key_config_1_ant = ' '
    key_config_2_ant = ' '
    key_config_3_ant = ' '
    for case in range(len(df_input)):                                      # loop for several configurations
        contract = df_input['contract'][case]
        time_ini = df_input['t_ini'][case]
        time_end = df_input['t_end'][case]
        year_ini = df_input['y_ini'][case]                                  # here I take the main variables
        year_end = df_input['y_end'][case]
        range_yrs = df_input['rng_years'][case]
        range_adj = df_input['rng_adjust'][case]
        ax_target = df_input['axis_target'][case]
        ax_limit = 1
        print(f'Processing {case} of {len(df_input)}')
        key_config_0 = {contract}
        if (not mac.validate_file(f'./Data/{contract}_00.csv')) | ((force == 'yes') & (key_config_0 != key_config_0_ant)):
            df_raw = mac.data_raw(contract)                          # XX_00
            my_time = datetime.datetime.now()
            print(f'{my_time} Log: raw data information in file {contract}.csv already imported and dataframe built')
        else:
            df_raw = pd.read_csv(f'./Data/{contract}_00.csv',
                                 dtype={'date': str, 'time': str, 'open': float, 'max': float, 'min': float,
                                        'close': float, 'volume': int, 'datetime_US': str, 'datetime_EU': str, },
                                 parse_dates=['datetime_US', 'datetime_EU'])
            my_time = datetime.datetime.now()
            print(f'{my_time} Log: raw data information in file {contract}_00.csv already imported in dataframe')
        key_config_0_ant = key_config_0

        key_config_1 = f'{contract}{time_ini[0:2]}{time_ini[3:]}{time_end[0:2]}{time_end[3:]}'
        if (not mac.validate_file(f'./Data/{key_config_1}_01.csv')) | ((force == 'yes') & (key_config_1 != key_config_1_ant)):
            df_day = mda.daily_range(df_raw, time_ini, time_end)
            df_day.to_csv(f'./Data/{key_config_1}_01.csv', index=False)                      # XX_01 daily with range
            my_time = datetime.datetime.now()
            print(f'{my_time} Log: information about ranges already treated in the file ./Data/{key_config_1}_01.csv')
        else:
            df_day = pd.read_csv(f'./Data/{key_config_1}_01.csv',
                                 dtype={'date': str, 'timetable': str, 'max': float, 'min': float, 'range': float},
                                 parse_dates=['date'])
            my_time = datetime.datetime.now()
            print(f'{my_time} Log: information about ranges imported from file ./Data/{key_config_1}_01.csv')
        key_config_1_ant = key_config_1

        zero = '0'
        key_config_2 = f'{contract}{time_ini[0:2]}{time_ini[3:]}{time_end[0:2]}{time_end[3:]}' \
                       f'{str(range_yrs)}{(str(range_adj)+zero)[2:4]}'
        if (not mac.validate_file(f'./Data/{key_config_2}_02.csv')) | ((force == 'yes') & (key_config_2 != key_config_2_ant)):
            df_day = mda.range_trail_estimation(contract, range_yrs, range_adj, 'Y', df_day)
            df_day.to_csv(f'./Data/{key_config_2}_02.csv', index=False)                 # XX_02 daily with range-trail
            my_time = datetime.datetime.now()
            print(f'{my_time} Log: information about range-trail treated in the file ./Data/{key_config_2}_02.csv')
        else:
            df_day = pd.read_csv(f'./Data/{key_config_2}_02.csv',
                                 dtype={'date': str, 'timetable': str, 'max': float, 'min': float, 'range': float,
                                        'range-trail': float, 'range-avg': float}, parse_dates=['date'])
            my_time = datetime.datetime.now()
            print(f'{my_time} Log: information about range-trail imported from file ./Data/{key_config_2}_02.csv')
        key_config_2_ant = key_config_2

        key_config_3 = f'{contract}{time_ini[0:2]}{time_ini[3:]}{time_end[0:2]}{time_end[3:]}' \
                       f'{str(range_yrs)}{(str(range_adj)+zero)[2:4]}'
        if (not mac.validate_file(f'./Data/{key_config_3}_03.csv')) | ((force == 'yes') & (key_config_3 != key_config_3_ant)):
            df_day, df_test = mop.market_operation(df_raw, df_day, time_ini, time_end)
            df_day.to_csv(f'./Data/{key_config_3}_03.csv', index=False)           # XX_03 daily with result operation
            df_test.to_csv(f'./Data/{key_config_3}_test03.csv')
            my_time = datetime.datetime.now()
            print(f'{my_time} Log: information about operation treated in the file ./Data/{key_config_3}_03.csv')
        else:
            df_day = pd.read_csv(f'./Data/{key_config_3}_03.csv',
                                 dtype={'date': str, 'timetable': str, 'max': float, 'min': float, 'range': float,
                                        'range-trail': float, 'range-avg': float, 'open': float, 'long-close': float,
                                        'short-close': float, 'close': float, 'long-rst': float, 'short-rst': float,
                                        'long-acc': float, 'short-acc': float},
                                 parse_dates=['date'])
            my_time = datetime.datetime.now()
            print(f'{my_time} Log: information about operation imported from file ./Data/{key_config_3}_03.csv')
        key_config_3_ant = key_config_3

        #my_multiplier, my_guarantee, my_delta_time = time_guarantee(contract, year_ini, year_end, df_day)

        #if (algorit == 'LLEX') | (algorit == ''):
        #    df_day = mlo.short_long_operationLLEX(contract, year_ini, year_end, df_day, ax_limit, ax_target,
        #                                          my_multiplier)
        #    df_day.to_csv(f'./Data/{contract}_04.csv', index=False)                                      # XX_04
        #elif algorit == 'LSEX':
        #    df_day = mlo.short_long_operationLSEX(contract, year_ini, year_end, df_day, ax_limit, ax_target,
        #                                          my_multiplier)
        #    df_day.to_csv(f'./Data/{contract}_04.csv', index=False)                                      # XX_04
        #elif algorit == 'FLIP':
        #    df_day = mlo.short_long_operationFLIP(contract, year_ini, year_end, df_day, my_multiplier)
        #    df_day.to_csv(f'./Data/{contract}_04.csv', index=False)                                      # XX_04
        #my_result = df_day[(df_day['result-amt'] != 0) & (df_day['result-accum-amt'] != 0)]['result-amt']
        #date_serie = df_day[(df_day['result-amt'] != 0) & (df_day['result-accum-amt'] != 0)]['date']
        #print(f'>>>>> date-serie datatype = {type(date_serie)}')
        #my_time = datetime.datetime.now()
        #print(f'{my_time} Log: information about market decision already treated in the file ./Data/{contract}_04.csv')
#####
        #df_mc_sim, df_mc_stat, df_mc_quart = mmo.monte_carlo_num(df_day, my_guarantee, my_delta_time)
        #df_mc_sim.to_csv(f'./Data/{contract}_05.csv', index=False)                                      # XX_05
        #df_mc_stat.to_csv(f'./Data/{contract}_06.csv', index=False)                                      # XX_06
        #df_mc_quart.to_csv(f'./Data/{contract}_07.csv', index=False)                                      # XX_07
####
        #mst.statis(my_result, my_guarantee, my_delta_time)
        #print(mst.monthly_table(my_result, date_serie, my_guarantee))
        #mst.plot_cap_evo(my_result, date_serie, my_guarantee)
        #mst.plot_mc_simul(df_mc_sim, my_result, date_serie, my_guarantee)
        #mst.plot_minmax_cap(df_mc_sim, my_result, date_serie, my_guarantee)
        #mst.plot_drawdown_amt(my_result, date_serie, my_guarantee)
        #mst.plot_drawdown_pct(my_result, date_serie, my_guarantee)
        #mst.plot_netpft_percentil(df_mc_quart)
        #mst.plot_annual_netpft_pct_percentil(df_mc_quart)
        #mst.plot_max_drawdown_pct_percentil(df_mc_quart)

        #sys.exit(f'Error: system force to exist and file/path <{file_input}> exists at {my_time}')


def time_guarantee(my_contract, year_ini, year_end, my_df):
    if my_contract == 'ES':
        my_guarantee = 12000 * 2                             # between 4000$ and 30000$
        my_multiplier = 50
        my_tic = 0.25
    if my_contract == 'YM':
        my_guarantee = 10000 * 2                             # between 3000$ and 25000$
        my_multiplier = 5
        my_tic = 1
    if my_contract == 'NQ':
        my_guarantee = 15000 * 2                             # between 5000$ and 45000$
        my_multiplier = 20
        my_tic = 0.25
    if my_contract == 'CL':
        my_guarantee = 5000 * 2                             # between 2000$ and 18000$
        my_multiplier = 1000
        my_tic = 0.01
    if my_contract == 'RTY':
        my_guarantee = 7000 * 2                             # between 2000$ and 20000$
        my_multiplier = 50
        my_tic = 1
    int_date_0 = year_ini * 10000 + 1 * 100 + 1              # the day d0 = date(2008, 8, 18)
    int_date_1 = year_end * 10000 + 12 * 100 + 31              # the day d0 = date(2008, 8, 18)
    int_date_01 = mda.locate_1st_day('Y', year_ini, my_df, 'date')
    int_date_11 = mda.locate_lst_day('Y', year_end, my_df, 'date')
    d1 = my_df['date'][int_date_11]
    d0 = my_df['date'][int_date_01]
    delta = d1 - d0
    delta_sec = delta.total_seconds()
    delta_years = delta_sec/(365*24*60*60)
    return my_multiplier, my_guarantee, delta_years, my_tic


if __name__ == '__main__':
    arguments = argument_parser()
    path = arguments.path
    algorit = arguments.algorit
    force = arguments.force
    if (force == 'force') | (force == 'FORCE'):
        force = 'yes'

    main(path)
