# 1. First calculate the daily range (max - min)
# 2. Calculate the average for the last years
# 3. Adjust the average range with a percentage
# 4. Round the final number to the next integer

import pandas as pd
import numpy as np
from datetime import date


# function to transform an integer YYYYMMDD into a datatype datetime format

def int2date(argdate: int) -> date:
    """
    If you have date as an integer, use this method to obtain a datetime.date object.
    -Parameters
    ----------
    argdate : int
      Date as a regular integer value (example: 20160618)

    -Returns
    -------
    dateandtime.date
      A date object which corresponds to the given value `argdate`.
    """
    year = int(argdate / 10000)
    month = int((argdate % 10000) / 100)
    day = int(argdate % 100)
    return date(year, month, day)


# function to locate the first date of a period (year or month) in a dataframe with a column named 'date'

def locate_1st_day(my_period, my_data, my_df, my_column_date):
    """
    This function locate the first date of a period (year or month) in a dataframe with a column named 'date'
    with datetime datatype
    -Parameters
    ----------
    my_period : year 'Y', month 'M', day 'D'
    my_data : is an integer with the information: year with format YYYY and should be greater than 2009
                                                  month with format YYYYMM and should be greater than 200900
    my_df : this is the dataframe with a column with datetime datatype
    my_column_date : this is the name of the column with datetime datatype and we use to locate a date
    (1st day of a period)
    -Returns
    -------
    An integer with the index position in the dataframe
    Any error return -1
    """
    if my_column_date not in my_df.columns:
        print('error column date does not exist')  # print
        return -1
    if my_column_date not in my_df.select_dtypes(['datetime64']).columns:
        print('error column date is not datetime datatype')  # print
        return -1
    if (my_period == 'Y') & ((isinstance(my_data, np.int64)) | (isinstance(my_data, int))) & \
            (my_data > 2009) & (my_data < 2050):
        date_to_find = my_data * 10000 + 100 + 1
    elif (my_period == 'M') & ((isinstance(my_data, np.int64)) | (isinstance(my_data, int))) & \
            (my_data > 201000) & (my_data < 205000):
        date_to_find = my_data * 100 + 1
    elif (my_period == 'D') & ((isinstance(my_data, np.int64)) | (isinstance(my_data, int))) & \
            (my_data > 20090100) & (my_data < 20500100):
        date_to_find = my_data
    else:
        print(f'1st error input or argument {my_period} {my_data} {isinstance(my_data, np.int64)} '
              f'{isinstance(my_data, int)} {my_column_date}')
        # print
        return -1

    for reg in range(len(my_df)):
        my_int_date = my_df[my_column_date][reg].year * 10000 + my_df[my_column_date][reg].month * 100 + \
                      my_df[my_column_date][reg].day
        if my_int_date == date_to_find:
            #            print(reg, my_df[my_column_date][reg])                                      #print
            return reg
        elif my_int_date > date_to_find:
            #            print(reg, my_df[my_column_date][reg])                                      #print
            return reg
    print(f'the data does not exist : {my_int_date}')  # print
    return -1


# function to locate the first date of a period (year or month) in a dataframe with a column named 'date'

def locate_lst_day (my_period, my_data, my_df, my_column_date):
    """
    This function locate the last date of a period (year or month) in a dataframe with a column named 'date'
     with datetime datatype
    -Parameters
    ----------
    my_period : year 'Y', month 'M'
    my_data : is an integer with the information: year with format YYYY and should be greater than 2009
                                                  month with format YYYYMM and should be greater than 200900
    my_df : this is the dataframe with a column called 'date' with datetime datatype
    my_column_date : this is the name of the column with datetime datatype and we use to locate a date
     (1st day of a period)
    -Returns
    -------
    An integer with the index position in the dataframe
    Any error return -1
    """
    if my_column_date not in my_df.columns:
        print('error column date does not exist')                                # print
        return -1
    if my_column_date not in my_df.select_dtypes(['datetime64']).columns:
        print('error column date is not datetime datatype')                                # print
        return -1
    if (my_period == 'Y') & ((isinstance(my_data, np.int64)) | (isinstance(my_data, int))) & \
            (my_data > 2009) & (my_data < 2050):
        date_to_find = my_data * 10000 + 100 + 1
    elif (my_period == 'M') & ((isinstance(my_data, np.int64)) | (isinstance(my_data, int))) & \
            (my_data > 200900) & (my_data < 205000):
        date_to_find = my_data * 100 + 1
    else:
        print(f'1st error input or argument {my_period} {my_data} {type(my_data)} {my_column_date}')
        # print
        return -1
    in_period = False
    for reg in range(len(my_df)):
        my_int_date = my_df[my_column_date][reg].year * 10000 + my_df[my_column_date][reg].month * 100 \
                    + my_df[my_column_date][reg].day
        if (my_period == 'Y') & (int(my_int_date/10000) >= my_data):
            if (in_period == True) & (int(my_int_date/10000) > my_data):
              # print(reg-1, my_df[my_column_date][reg-1], int(my_int_date/10000))                      #print
              # print(reg, my_df[my_column_date][reg], int(my_int_date/10000))                        #print
                return reg-1
            in_period = True
        elif (my_period == 'M') & (int(my_int_date/100) >= my_data):
            if (in_period == True) & (int(my_int_date/100) > my_data):
              # print(reg-1, my_df[my_column_date][reg-1], int(my_int_date/100))                    #print
                return reg-1
            in_period=True
#    print(f'the data does not exist : {my_int_date}')                                  #print
    if in_period:
        # print('returned last register')
        return reg
    return -1


# function to go throughout the dataframe and calculate the daily range

def daily_range(data_rd, timetable_ini, timetable_end):
    """
    This function create a dataframe with maximum minimum and range for a daily base in a specific timetable
    -Parameters
    ----------
    data_rd : dataframe with original data (intraday movements of a future contract) ['date', 'time', 'open',
                'max', 'min', 'close', 'volume', 'datetime_US', 'datetime_EU']
    data_wr : new dataframe to create with columns ['date', 'timetable', 'max', 'min', 'range']. It must be empty
    timetable_ini : time the operation start everyday. Example='09:00'
    timetable_end : time the operation finish everyday. Example='18:00'

    -Returns
    -------
    nothing
    The function fill the dataframe data_wr with daily maximum, minimum, range(maximum-minimum)
    """
    data_wr = pd.DataFrame(columns=['date', 'timetable', 'max', 'min', 'range'])
    #  data_wr['date']=pd.to_datetime(data_wr['date'], format='%Y-%m-%d')

    hour_ini = int(timetable_ini[0:2])  # initial values
    minu_ini = int(timetable_ini[3:])
    hour_end = int(timetable_end[0:2])
    minu_end = int(timetable_end[3:])
    tmtb_ini = hour_ini * 100 + minu_ini  # integer value
    tmtb_end = hour_end * 100 + minu_end  # integer value
    minim = 0.0
    maxim = 999999.9
    flag_in = False
    my_dt_ant = 20000101
    j = 0
    #    print(tmtb_ini, tmtb_end)
    for i in range(len(data_rd)):
        my_dt = data_rd['datetime_EU'][i].year * 10000 + data_rd['datetime_EU'][i].month * 100 + \
                data_rd['datetime_EU'][i].day
        my_ho = data_rd['datetime_EU'][i].hour
        my_mi = data_rd['datetime_EU'][i].minute
        my_tmtb = my_ho * 100 + my_mi  # timetable as integer
        #        print(my_dt, my_tmtb)
        if my_dt_ant == my_dt:
            if (my_tmtb == tmtb_ini) & (flag_in == False):  # the start of timetable
                maxim = data_rd['max'][i]
                minim = data_rd['min'][i]
                flag_in = True
            elif (my_tmtb > tmtb_ini) & (my_tmtb < tmtb_end) & (flag_in == True):  # update max and min values
                if data_rd['max'][i] > maxim:
                    maxim = data_rd['max'][i]
                if data_rd['min'][i] < minim:
                    minim = data_rd['min'][i]
            elif (my_tmtb == tmtb_end) & (flag_in == True):  # the end of timetable and save daily values
                flag_in = False
                my_dt_ant_date = int2date(my_dt_ant)
                data_wr.loc[j] = [my_dt_ant_date, timetable_ini + '-' + timetable_end, maxim, minim, maxim - minim]
                j += 1
                maxim = 99999.9
                minim = 0.0
        #            print(i, len(data_rd), maxim, minim)
        my_dt_ant = my_dt
    data_wr['date'] = pd.to_datetime(data_wr['date'], format='%Y-%m-%d')
    return data_wr


# function to go throughout the dataframe and calculate the daily range

def range_trail_estimation(my_contract, years, adjustment, updating, my_df):
    """
    This function estimate and create the range-trail to use in the daily operation in the market according to
    the following parameters:
    Then the data frame df_es_day will be incremented with a new column 'trail' with the daily value
    or estimated range-trail
    -Parameters
    ----------
    years (int): the quantity of years we are going to use to calculate the average. For example 4 years means
                that we will take the last 4 years to calculate the average range.
    adjustment (float): the percentage (%) we will apply to adjust the average range calculated. For example 0.70 (70%).
    updating (string): this is the time we are going to update the range-trail estimation.
                    For example 'Y' means every year.
            From 1st of January we will use the range-trail estimated until the 31th of December.

            formula >> range_trail = (average of range of last 4 years) * (adjustment)
                    >> then round up to the closer integer, tenth, hundredth, thousandth according to the contract value
    my_df : this is the dataframe to work with and its columns ['date', 'timetable', 'max', 'min', 'range']
            we will create a new column range-trail
    -Returns
    -------
    Return de dataframe updated my_df
    The function fill the dataframe my_df with daily range-trail
    """
    pd.options.mode.chained_assignment = None  # default='warn'   This is to avoid warnings of chained-assignment

    if years not in [1, 2, 3, 4, 5]:
        print('Error: Only 1, 2, 3, 4 or 5 years')
        return -1
    if (adjustment < 0.65) & (adjustment > 0.80):
        print('Error: only 0.65, 0.70, 0.75 or 0.80 value for adjustment')
        return -1
    if updating != 'Y':
        print('Error: only Y for updating')
        return -1

    my_df['range-trail'] = np.zeros(len(my_df))
    my_df['range-avg'] = np.zeros(len(my_df))
    # my_df = my_df.reindex(columns = ['date', 'timetable', 'max', 'min', 'range', 'range-trail'])

    first_year = my_df.iloc[0, :]['date'].year
    last_year = my_df.iloc[-1, :]['date'].year
    year_in = first_year + years + 1
    year_out = last_year

    total_loops = year_out - year_in + 1                               # 2019 - 2014 = 5 + 1
    loop_num = 0

    while loop_num < total_loops:

        year_to_start = year_in + loop_num - years                            # 2015 + loop - 4 = 2010
        year_to_finish = year_out                                              # 2019

        pta_ini = locate_1st_day('Y', year_to_start, my_df, 'date')                  # 2010
        pta_end = locate_lst_day('Y', year_to_start + years - 1, my_df, 'date')        # 2010 + 4 -1 = 2013
        my_range_sum = 0.0
        #        print(year_to_start, pta_ini, pta_end)
        for i in range(pta_ini, pta_end + 1):
            my_range_sum += my_df['range'][i]

        my_range_avg = my_range_sum / (pta_end - pta_ini)
        my_range_trl = trail_round(my_range_avg, adjustment, my_contract)

        ptt_ini = locate_1st_day('Y', year_to_start + years, my_df, 'date')                 # 2010 + 4 = 2014
        ptt_end = locate_lst_day('Y', year_to_start + years, my_df, 'date')                 # 2010 + 4 = 2014
        #        print(year_to_start+years, ptt_ini, ptt_end)
        for i in range(ptt_ini, ptt_end + 1):
            my_df['range-trail'][i] = my_range_trl
            my_df['range-avg'][i] = my_range_avg

        loop_num += 1

    return my_df


def trail_round(my_avg, my_adj, my_contract):
    my_round = 0
    if my_contract == 'ES':
        my_round = round(my_avg * my_adj + 0.5, 0) + 1                # Round to units
    if my_contract == 'NQ':
        my_round = round(my_avg * my_adj + 0.5, 0) + 1                # Round to units
    if my_contract == 'YM':
        my_round = round(my_avg * my_adj + 5, -1) + 10             # Round to tens
    if my_contract == 'CL':
        my_round = round(my_avg * my_adj + 0.05, 1) + 0.1                # Round to tenths
    if my_contract == 'RTY':
        my_round = round(my_avg * my_adj + 0.5, 0) + 1                # Round to units

    return my_round
