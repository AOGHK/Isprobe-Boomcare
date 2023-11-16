package com.physi.dev.nursinglight.list;

import android.view.View;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.physi.dev.nursinglight.R;

public class WiFiItemHolder extends RecyclerView.ViewHolder {

    protected TextView tvName;

    public WiFiItemHolder(@NonNull View itemView) {
        super(itemView);

        tvName = itemView.findViewById(R.id.tv_wifi_item);
    }
}
