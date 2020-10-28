import pandas as pd
import numpy as np
import datetime
from datetime import date
from p_ranging import m_dataranging as mda               # import from pack ranging the module


# function to follow the daily operation and take the decision about operation long or short
# system market follow long accumulated ratio with limit and target.


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

    pt_ini = mda.locate_1st_day('Y', year_ini, my_df, 'date')  # locate the initial index
    pt_end = mda.locate_lst_day('Y', year_end, my_df, 'date')  # locate the final index

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
            result_accum_pts = result_accum_pts + long_result - tic
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
            result_accum_pts = result_accum_pts + short_result - tic
            result_accum_amt = result_accum_amt + short_result * contract_qty * my_multiplier

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


# function to follow the daily operation and take the decision about operation long or short
# market system to replicate the day before according to the performance.

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

    pt_ini = mda.locate_1st_day('Y', year_ini, my_df, 'date')  # locate the initial index
    pt_end = mda.locate_lst_day('Y', year_end, my_df, 'date')  # locate the final index

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
            result_accum_pts = result_accum_pts + long_result - tic
            result_accum_amt = result_accum_amt + (long_result - tic) * contract_qty * my_multiplier

            if (long_result < 0) & (continue_no_changes == 1):        # bad long performance and change to shorts
                operation_ant = long
                operation_nxt = short
                continue_no_changes = 0

        if operation == short:
            result_accum_pts = result_accum_pts + short_result - tic
            result_accum_amt = result_accum_amt + short_result * contract_qty * my_multiplier

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


# function to follow the daily operation and take the decision about operation long or short
# market system to follow long/short accumulated ratio with limit and target.

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

    pt_ini = mda.locate_1st_day('Y', year_ini, my_df, 'date')  # locate the initial index
    pt_end = mda.locate_lst_day('Y', year_end, my_df, 'date')  # locate the final index

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
            result_accum_pts = result_accum_pts + long_result - tic
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
            result_accum_pts = result_accum_pts + short_result - tic
            result_accum_amt = result_accum_amt + short_result * contract_qty * my_multiplier

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
