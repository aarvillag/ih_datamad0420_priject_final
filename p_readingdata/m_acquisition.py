# Notebook for date-time formatting from original dataset
# We will create two additional columns with datetime in US and EU format

import pandas as pd
import os.path


def validate_file(pathfile):                                # Checking if contract file exists
    if os.path.isfile(f'./{pathfile}'):
        return True
    else:
        return False


def data_input (file_input):                   # load file input and create a dataframe and validation
    pd.options.mode.chained_assignment = None   # default='warn'   This is to avoid warnings of chained-assignment
    my_df_i = pd.read_csv(file_input, skiprows=1, names=['contract', 't_ini', 't_end', 'rng_years',
                                                         'rng_adjust', 'axis_target', 'y_ini', 'y_end'],
                          dtype={'contract': str, 't_ini': str, 't_end': str, 'y_ini': int, 'y_end': int,
                                 'rng_years': int, 'rng_adjust': float, 'axis_target': int})
    my_err=''
    for i in range(len(my_df_i)):
        my_df_i['contract'][i]=my_df_i['contract'][i].upper()
        if my_df_i['contract'][i] not in ['ES', 'YM', 'NQ', 'CL']:
            my_err='contract not valid (only ES, YM, NQ or CL'
        if my_df_i['t_ini'][i] not in ['01:00','09:00','15:00']:
            my_err='initial timetable not valid (only 01:00,09:00 or 15:00)'
        if my_df_i['t_end'][i] not in ['08:30','18:30','22:30']:
            my_err='final timetable not valid (only 08:30,18:30 or 22:30)'
        my_ini_time = int(my_df_i['t_ini'][i][0:2]) * 100 + int(my_df_i['t_ini'][i][4:])
        my_end_time = int(my_df_i['t_end'][i][0:2]) * 100 + int(my_df_i['t_end'][i][4:])
        if my_end_time <= my_ini_time:
            my_err = f'timetable not valid ({my_ini_time} - {my_end_time})'
        if my_df_i['y_ini'][i] not in [2014, 2015, 2016, 2017, 2018, 2019, 2020]:
            my_err = 'initial year not valid (2014, 2015, 2016, 2017, 2018, 2019, 2020)'
        if my_df_i['y_end'][i] not in [2014, 2015, 2016, 2017, 2018, 2019, 2020]:
            my_err = 'final year not valid (2014, 2015, 2016, 2017, 2018, 2019, 2020)'
        if my_df_i['y_end'][i] < my_df_i['y_ini'][i]:
            my_err = 'final year is not valid, it mus be grater than initial year'
        if my_df_i['rng_years'][i] not in [1, 2, 3, 4, 5]:
            my_err='years for range is not valid (only 1, 2, 3, 4, 5)'
        if my_df_i['rng_adjust'][i] not in [0.65, 0.70, 0.75, 0.80]:
            my_err = 'the percentage for range adjustment is not valid (only 0.65, 0.70, 0.75, 0.80)'
        if my_df_i['axis_target'][i] not in [2, 3, 4, 5]:
            my_err = 'the qty axes for target is not valid (only 2, 3, 4, 5)'
    return my_df_i, my_err


def data_raw(contract):
    df = pd.read_csv(f'./{contract}.csv', header=0, names=['date', 'time', 'open', 'max', 'min', 'close', 'volume'])
    df['datetime_US'] = pd.to_datetime(df[['date', 'time']].agg(' '.join, axis=1), format='%m/%d/%Y %H:%M')
    df['datetime_EU'] = df['datetime_US'] + pd.to_timedelta('6 h')
    df.to_csv(f'./Data/{contract}_00.csv', index=False)
    return df


