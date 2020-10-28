import pandas as pd
import numpy as np
from datetime import date
import streamlit as st
import os.path

# YOU CAN FIND THE FOLLOWING FUNCTIONS:
#
# max_dd_evol(guarantee, my_serie)
# max_dd(guarantee, my_serie)
# int2date(argdate: int)
# locate_1st_day(my_period, my_data, my_df, my_column_date)
# locate_lst_day (my_period, my_data, my_df, my_column_date)
# time_guarantee(my_contract, year_ini, year_end, my_df)
#


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
    if (my_period == 'Y') & ((isinstance(my_data, np.int64)) | (isinstance(my_data, int))) & (my_data > 2009) & (my_data < 2050):
        date_to_find = my_data * 10000 + 100 + 1
    elif (my_period == 'M') & ((isinstance(my_data, np.int64)) | (isinstance(my_data, int))) & (my_data > 201000) & (my_data < 205000):
        date_to_find = my_data * 100 + 1
    elif (my_period == 'D') & ((isinstance(my_data, np.int64)) | (isinstance(my_data, int))) & (my_data > 20100100) & (my_data < 20500100):
        date_to_find = my_data
    else:
        print(f'1st error input or argument {my_period} {my_data} {isinstance(my_data, np.int64)} {my_column_date}')        # print
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
        print('error column date does not exist')                                #print
        return -1
    if my_column_date not in my_df.select_dtypes(['datetime64']).columns:
        print('error column date is not datetime datatype')                                #print
        return -1
    if (my_period == 'Y') & ((isinstance(my_data, np.int64)) | (isinstance(my_data, int))) & (my_data > 2009) & (my_data < 2050):
        date_to_find = my_data * 10000 + 100 + 1
    elif (my_period == 'M') & ((isinstance(my_data, np.int64)) | (isinstance(my_data, int))) & (my_data > 200900) & (my_data < 205000):
        date_to_find = my_data * 100 + 1
    else:
        print(f'1st error input or argument {my_period} {my_data} {isinstance(my_data, np.int64)} {type(my_data)} {my_column_date}')
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
        print('returned last register')
        return reg
    return -1

######################################################################################33

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
        my_tic = 1
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
    int_date_01 = locate_1st_day('Y', year_ini, my_df, 'date')
    int_date_11 = locate_lst_day('Y', year_end, my_df, 'date')
    d1 = my_df['date'][int_date_11]
    d0 = my_df['date'][int_date_01]
    delta = d1 - d0
    delta_sec = delta.total_seconds()
    delta_years = delta_sec/(365*24*60*60)
    return my_multiplier, my_guarantee, delta_years, my_tic


###########################################################################33

def short_long_operationLLEX(my_contract, year_ini, year_end, my_df, limit_qty, target_qty, my_multiplier):
    pd.options.mode.chained_assignment = None  # default='warn' This is to avoid warnings of chained-assignment

    # creation of new columns in datframe
    my_df['operation'] = ' '
    my_df['target'] = np.zeros(len(my_df)).astype('float')
    my_df['limit'] = np.zeros(len(my_df)).astype('float')
    my_df['result-pts'] = np.zeros(len(my_df)).astype('float')
    my_df['result-accum-pts'] = np.zeros(len(my_df)).astype('float')
    my_df['contract-qty'] = np.zeros(len(my_df)).astype('float')
    my_df['multiplier'] = np.zeros(len(my_df)).astype('float')
    my_df['result-amt'] = np.zeros(len(my_df)).astype('float')
    my_df['result-accum-amt'] = np.zeros(len(my_df)).astype('float')

    # define variables to use
    long = 'L'  #
    short = 'S'  #
    operation_ant = long  #
    limit_ant = 0.0  #
    target_ant = 0.0  #
    limit_qty_rng = limit_qty    ##########
    target_qty_rng = target_qty  ############
    axis_size = 0.0  #
    tic = 0.0  #
    contract_qty = 1.0  #

    # define variable for operation

    long_accum = 0.0  #
    short_accum = 0.0  #
    result_accum_amt = 0.0  #
    result_accum_pts = 0.0  #
    limit_nxt = 0 - limit_qty_rng * axis_size  #
    target_nxt = 0 + target_qty_rng * axis_size  #
    operation_nxt = long  #
    target_reached = 0  #

    # locate the initial and final position

    pt_ini = locate_1st_day('Y', year_ini, my_df, 'date')  # locate the initial index *****************
    pt_end = locate_lst_day('Y', year_end, my_df, 'date')  # locate the final index *****************

    # the loop

    for ind in range(pt_ini, pt_end + 1):
        range_trail = my_df['range-trail'][ind]
        long_result = my_df['long-rst'][ind]
        short_result = my_df['short-rst'][ind]

        axis_size = range_trail

        if ind == pt_ini:
            limit_nxt = 0 - limit_qty_rng * axis_size
            target_nxt = 0 + target_qty_rng * axis_size

        operation = operation_nxt
        limit = limit_nxt
        target = target_nxt

        long_accum = long_accum + long_result
        short_accum = short_accum + short_result

        continue_no_changes = 1

        if operation == long:
            result_accum_pts = result_accum_pts + long_result
            result_accum_amt = result_accum_amt + (long_result - tic) * contract_qty * my_multiplier

            if (long_accum < limit) & (continue_no_changes == 1):  # overcome the limit and change to shorts
                operation_ant = long
                operation_nxt = short
                limit_ant = limit
                limit_nxt = long_accum + (limit_qty_rng * axis_size)
                target_ant = target
                target_nxt = long_accum - (target_qty_rng * axis_size)
                continue_no_changes = 0

            if ((target_reached == 1) & (long_result <= ((0 - 1) * range_trail * 0.2)) &
                    (continue_no_changes == 1)):  # target reached with reverse of 20%
                operation_ant = long
                operation_nxt = short
                limit_ant = limit
                limit_nxt = long_accum + (limit_qty_rng * axis_size)
                target_ant = target
                target_nxt = long_accum - (target_qty_rng * axis_size)
                target_reached = 0
                continue_no_changes = 0

            if ((long_accum > target) & (target_reached == 0) &
                    (continue_no_changes == 1)):  # target reached but waiting reverse
                target_reached = 1
                operation_ant = operation
                operation_nxt = operation_ant
                limit_ant = limit
                limit_nxt = limit_ant
                target_ant = target
                target_nxt = target_ant
                continue_no_changes = 0

        if operation == short:
            result_accum_pts = result_accum_pts + short_result
            result_accum_amt = result_accum_amt + (short_result - tic) * contract_qty * my_multiplier

            if (long_accum > limit) & (continue_no_changes == 1):  # overcome limit and change
                operation_ant = short
                operation_nxt = long
                limit_ant = limit
                limit_nxt = long_accum - (limit_qty_rng * axis_size)
                target_ant = target
                target_nxt = long_accum + (target_qty_rng * axis_size)
                continue_no_changes = 0

            if ((target_reached == 1) & (long_result > (range_trail * 0.2)) &
                    (continue_no_changes == 1)):  # target reached with reverse of 20% and change
                operation_ant = short
                operation_nxt = long
                limit_ant = limit
                limit_nxt = long_accum - (limit_qty_rng * axis_size)
                target_ant = target
                target_nxt = long_accum + (target_qty_rng * axis_size)
                target_reached = 0
                continue_no_changes = 0

            if ((long_accum < target) & (target_reached == 0) &
                    (continue_no_changes == 1)):  # target reached but waiting reverse
                target_reached = 1
                operation_ant = operation
                operation_nxt = operation_ant
                limit_ant = limit
                limit_nxt = limit
                target_ant = target
                target_nxt = target
                continue_no_changes = 0

        if continue_no_changes == 1:  # nothing happen, no target reached, no limit overcame, continue the same
            operation_ant = operation
            operation_nxt = operation_ant
            limit_ant = limit
            limit_nxt = limit_ant
            target_ant = target
            target_nxt = target_ant

        # save the daily operation

        my_df['operation'][ind] = operation_ant
        my_df['target'][ind] = target_ant
        my_df['limit'][ind] = limit_ant
        my_df['result-pts'][ind] = result_accum_pts - my_df['result-accum-pts'][ind - 1]
        my_df['result-accum-pts'][ind] = result_accum_pts
        my_df['contract-qty'][ind] = contract_qty
        my_df['multiplier'][ind] = my_multiplier
        my_df['result-amt'][ind] = result_accum_amt - my_df['result-accum-amt'][ind - 1]
        my_df['result-accum-amt'][ind] = result_accum_amt

    return my_df


def short_long_operationFLIP( my_contract, year_ini, year_end, my_df, my_multiplier):
    pd.options.mode.chained_assignment = None  # default='warn' This is to avoid warnings of chained-assignment

    # creation of new columns in datframe
    my_df['operation'] = ' '
    my_df['target'] = np.zeros(len(my_df)).astype('float')
    my_df['limit'] = np.zeros(len(my_df)).astype('float')
    my_df['result-pts'] = np.zeros(len(my_df)).astype('float')
    my_df['result-accum-pts'] = np.zeros(len(my_df)).astype('float')
    my_df['contract-qty'] = np.zeros(len(my_df)).astype('float')
    my_df['multiplier'] = np.zeros(len(my_df)).astype('float')
    my_df['result-amt'] = np.zeros(len(my_df)).astype('float')
    my_df['result-accum-amt'] = np.zeros(len(my_df)).astype('float')

    # define variables to use
    long = 'L'  #
    short = 'S'  #
    operation_ant = long  #
    tic = 0.0  #
    contract_qty = 1.0         #


    # define variable for operation

    long_accum = 0.0  #
    short_accum = 0.0  #
    result_accum_amt = 0.0  #
    result_accum_pts = 0.0  #
    operation_nxt = long  #

    # locate the initial and final position

    pt_ini = locate_1st_day('Y', year_ini, my_df, 'date')  # locate the initial index
    pt_end = locate_lst_day('Y', year_end, my_df, 'date')  # locate the final index

    # the loop

    for ind in range(pt_ini, pt_end + 1):

        range_trail = my_df['range-trail'][ind]
        long_result = my_df['long-rst'][ind]
        short_result = my_df['short-rst'][ind]

        axis_size = range_trail

        if ind == pt_ini:
            operation_nxt = long

        operation = operation_nxt

        long_accum = long_accum + long_result
        short_accum = short_accum + short_result

        continue_no_changes = 1

        if operation == long:
            result_accum_pts = result_accum_pts + long_result
            result_accum_amt = result_accum_amt + (long_result - tic) * contract_qty * my_multiplier

            if (long_result < 0) & (continue_no_changes == 1):        # bad long performance and change to shorts
                operation_ant = long
                operation_nxt = short
                continue_no_changes = 0

        if operation == short:
            result_accum_pts = result_accum_pts + short_result
            result_accum_amt = result_accum_amt + (short_result - tic) * contract_qty * my_multiplier

            if (short_result < 0) & (continue_no_changes == 1):          # bad short performance and change to long
                operation_ant = short
                operation_nxt = long
                continue_no_changes = 0

        if continue_no_changes == 1:  # nothing happen, continue the same
            operation_ant = operation
            operation_nxt = operation_ant

        # save the daily operation

        my_df['operation'][ind] = operation_ant
        my_df['target'][ind] = 0
        my_df['limit'][ind] = 0
        my_df['result-pts'][ind] = result_accum_pts - my_df['result-accum-pts'][ind - 1]
        my_df['result-accum-pts'][ind] = result_accum_pts
        my_df['contract-qty'][ind] = contract_qty
        my_df['multiplier'][ind] = my_multiplier
        my_df['result-amt'][ind] = result_accum_amt - my_df['result-accum-amt'][ind - 1]
        my_df['result-accum-amt'][ind] = result_accum_amt

    return my_df



def short_long_operationLSEX (my_contract, year_ini, year_end, my_df, limit_qty, target_qty, my_multiplier):
    pd.options.mode.chained_assignment = None  # default='warn' This is to avoid warnings of chained-assignment

    # creation of new columns in datframe
    my_df['operation'] = ' '
    my_df['target'] = np.zeros(len(my_df)).astype('float')
    my_df['limit'] = np.zeros(len(my_df)).astype('float')
    my_df['result-pts'] = np.zeros(len(my_df)).astype('float')
    my_df['result-accum-pts'] = np.zeros(len(my_df)).astype('float')
    my_df['contract-qty'] = np.zeros(len(my_df)).astype('float')
    my_df['multiplier'] = np.zeros(len(my_df)).astype('float')
    my_df['result-amt'] = np.zeros(len(my_df)).astype('float')
    my_df['result-accum-amt'] = np.zeros(len(my_df)).astype('float')

    # define variables to use
    long = 'L'  #
    short = 'S'  #
    operation = long  #
    operation_ant = long  #
    limit = 0.0  #
    limit_ant = 0.0  #
    target = 0.0  #
    target_ant = 0.0  #
    limit_qty_rng = limit_qty  ##########
    target_qty_rng = target_qty  ############
    range_trail = 0.0  #
    axis_size = 0.0  #
    tic = 0.0  #
    contract_qty = 1.0  #

    # define variable for operation

    long_accum = 0.0  #
    short_accum = 0.0  #
    result_accum_amt = 0.0  #
    result_accum_pts = 0.0  #
    limit_nxt = 0 - limit_qty_rng * axis_size  #
    target_nxt = 0 + target_qty_rng * axis_size  #
    operation_nxt = long  #
    target_reached = 0  #

    # locate the initial and final position

    pt_ini = locate_1st_day('Y', year_ini, my_df, 'date')  # locate the initial index
    pt_end = locate_lst_day('Y', year_end, my_df, 'date')  # locate the final index

    # the loop

    for ind in range(pt_ini, pt_end + 1):
        range_trail = my_df['range-trail'][ind]
        long_result = my_df['long-rst'][ind]
        short_result = my_df['short-rst'][ind]

        axis_size = range_trail

        if ind == pt_ini:
            limit_nxt = 0 - limit_qty_rng * axis_size
            target_nxt = 0 + target_qty_rng * axis_size

        operation = operation_nxt
        limit = limit_nxt
        target = target_nxt

        long_accum = long_accum + long_result
        short_accum = short_accum + short_result

        continue_no_changes = 1

        if operation == long:
            result_accum_pts = result_accum_pts + long_result
            result_accum_amt = result_accum_amt + (long_result - tic) * contract_qty * my_multiplier

            if (long_accum < limit) & (continue_no_changes == 1):  # overcome the limit and change to shorts
                operation_ant = long
                operation_nxt = short
                limit_ant = limit
                limit_nxt = short_accum - (limit_qty_rng * axis_size)
                target_ant = target
                target_nxt = short_accum + (target_qty_rng * axis_size)
                continue_no_changes = 0

            if ((target_reached == 1) & (long_result <= ((0 - 1) * range_trail * 0.2)) &
                    (continue_no_changes == 1)):  # target reached with reverse of 20%
                operation_ant = long
                operation_nxt = short
                limit_ant = limit
                limit_nxt = short_accum - (limit_qty_rng * axis_size)
                target_ant = target
                target_nxt = short_accum + (target_qty_rng * axis_size)
                target_reached = 0
                continue_no_changes = 0

            if ((long_accum > target) & (target_reached == 0) & (
                    continue_no_changes == 1)):  # target reached but waiting reverse
                target_reached = 1
                operation_ant = operation
                operation_nxt = operation_ant
                limit_ant = limit
                limit_nxt = limit_ant
                target_ant = target
                target_nxt = target_ant
                continue_no_changes = 0

        if operation == short:
            result_accum_pts = result_accum_pts + short_result
            result_accum_amt = result_accum_amt + (short_result - tic) * contract_qty * my_multiplier

            if (short_accum < limit) & (continue_no_changes == 1):  # overcome limit and change
                operation_ant = short
                operation_nxt = long
                limit_ant = limit
                limit_nxt = long_accum - (limit_qty_rng * axis_size)
                target_ant = target
                target_nxt = long_accum + (target_qty_rng * axis_size)
                continue_no_changes = 0

            if ((target_reached == 1) & (short_result <= ((0 - 1) * range_trail * 0.2)) & (
                    continue_no_changes == 1)):  # target reached with reverse of 20% and change
                operation_ant = short
                operation_nxt = long
                limit_ant = limit
                limit_nxt = long_accum - (limit_qty_rng * axis_size)
                target_ant = target
                target_nxt = long_accum + (target_qty_rng * axis_size)
                target_reached = 0
                continue_no_changes = 0

            if ((short_accum > target) & (target_reached == 0) & (
                    continue_no_changes == 1)):  # target reached but waiting reverse
                target_reached = 1
                operation_ant = operation
                operation_nxt = operation_ant
                limit_ant = limit
                limit_nxt = limit
                target_ant = target
                target_nxt = target
                continue_no_changes = 0

        if continue_no_changes == 1:  # nothing happen, no target reached, no limit overcame, continue the same
            operation_ant = operation
            operation_nxt = operation_ant
            limit_ant = limit
            limit_nxt = limit_ant
            target_ant = target
            target_nxt = target_ant

        # save the daily operation

        my_df['operation'][ind] = operation_ant
        my_df['target'][ind] = target_ant
        my_df['limit'][ind] = limit_ant
        my_df['result-pts'][ind] = result_accum_pts - my_df['result-accum-pts'][ind - 1]
        my_df['result-accum-pts'][ind] = result_accum_pts
        my_df['contract-qty'][ind] = contract_qty
        my_df['multiplier'][ind] = my_multiplier
        my_df['result-amt'][ind] = result_accum_amt - my_df['result-accum-amt'][ind - 1]
        my_df['result-accum-amt'][ind] = result_accum_amt

    return my_df

###############################################3

def validate_file(pathfile):                                # Checking if file exists
    if os.path.isfile(f'./{pathfile}'):
        return True
    else:
        return False

