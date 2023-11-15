package com.physi.dev.nursinglight.list;

import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.physi.dev.nursinglight.R;

public class BleHolder extends RecyclerView.ViewHolder {

    protected LinearLayout item;
    protected TextView tvName, tvAddress;

    public BleHolder(@NonNull View itemView) {
        super(itemView);

        item = itemView.findViewById(R.id.item_ble_device);
        tvName = itemView.findViewById(R.id.tv_ble_name);
        tvAddress = itemView.findViewById(R.id.tv_ble_address);
    }
}
