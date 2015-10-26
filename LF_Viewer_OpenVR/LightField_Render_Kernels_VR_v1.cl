/**********************************************************************
Author: Yu Ji ?015 
Light Field Rendering on OpenCL


********************************************************************/

__kernel void LightField_ray_Render(__global uchar4* lf_src, 
									__global uchar* lf_dep,
									__read_only image2d_t depthmap,
									__write_only image2d_t nview_dst, 

									int lfcam_res, 

									int lfcam_num_height, 
									int lfcam_num_width,
									float lfcam_fov,
									
									int nview_res_w,


									float nview_focal_length,

									float4 nview_look_at,
									float4 nview_up,
									float4 nview_right,
									float4 nview_pos,
									
									int color_coded_slabs,
									float depth_scal,
									int show_depth,
									float shear_hor,
									float shear_ver)
{
	const sampler_t SAMPLER = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

	int2 locate_dst;
	locate_dst.x = get_global_id(0); // final pixel location 
	locate_dst.y = get_global_id(1);



	float4 t_depth;
	t_depth = read_imagef( depthmap, SAMPLER, locate_dst );

	/*********  convert pixel to ray  *********/

	const int icol = get_global_id(0);  // first try 0 is col, 1 is row!!!
	const int irow = get_global_id(1);

	

	float row_norm;
	float col_norm;

	row_norm = (float)irow/(float)(nview_res_w-1) -0.619289339 ;
	col_norm = (float)icol/(float)(nview_res_w-1) -0.5;
	

	float xcen = nview_pos.s0;
	float ycen = nview_pos.s1;
	float zcen = nview_pos.s2;

	float px = nview_pos.s0+nview_look_at.s0 * nview_focal_length;
	float py = nview_pos.s1+nview_look_at.s1 * nview_focal_length;
	float pz = nview_pos.s2+nview_look_at.s2 * nview_focal_length;

	float4 dir;

	dir.x = px + nview_right.s0 * col_norm + nview_up.s0 * row_norm; // 1st row is bottom-most, 1st col is left-most
	dir.y = py + nview_right.s1 * col_norm + nview_up.s1 * row_norm;
	dir.z = pz + nview_right.s2 * col_norm + nview_up.s2 * row_norm;

	dir.x = dir.x - xcen;
	dir.y = dir.y - ycen;
	dir.z = dir.z - zcen;

	//float4 nview_look_at_norm = normalize(nview_look_at);
	float4 nview_look_at_norm = nview_look_at;

	dir = normalize(dir);

	float cos_angle = dot(nview_look_at_norm,dir);

	float dx = dir.x;
	float dy = dir.y;
	float dz = dir.z;

	


	/*********  detect ray face intersection  *********/

	float ang = atan2pi(dx,dz)+0.75;
	int fnum;

	float abs_dx;
	float abs_dz;
	if(dx < 0)
	{
		abs_dx = -dx;
	}
	else
	{
		abs_dx = dx;
	}

	if(dz < 0)
	{
		abs_dz = -dz;
	}
	else
	{
		abs_dz = dz;
	}
	

	if(dy > abs_dx && dy > abs_dz)
	{
		fnum = 5;
	}
	else if(dy < -abs_dx && dy < -abs_dz)
	{
		fnum = 6;
	}
	else
	{
		if(ang < 0)
		{
			ang += 2;
		}

		fnum = 5 - ceil(ang*2);
	}


	/*********  convert to face 1  *********/


	if(fnum == 2)
	{
		float xcen_new = zcen;
		float ycen_new = ycen;
		float zcen_new = -xcen;

		xcen = xcen_new;
		ycen = ycen_new;
		zcen = zcen_new;

		float dx_new = dz;
		float dy_new = dy;
		float dz_new = -dx;

		dx = dx_new;
		dy = dy_new;
		dz = dz_new;
	}
	else if(fnum == 3)
	{
		float xcen_new = -xcen;
		float ycen_new = ycen;
		float zcen_new = -zcen;

		xcen = xcen_new;
		ycen = ycen_new;
		zcen = zcen_new;

		float dx_new = -dx;
		float dy_new = dy;
		float dz_new = -dz;

		dx = dx_new;
		dy = dy_new;
		dz = dz_new;
	}
	else if(fnum == 4)
	{
		float xcen_new = -zcen;
		float ycen_new = ycen;
		float zcen_new = xcen;

		xcen = xcen_new;
		ycen = ycen_new;
		zcen = zcen_new;

		float dx_new = -dz;
		float dy_new = dy;
		float dz_new = dx;

		dx = dx_new;
		dy = dy_new;
		dz = dz_new;
	}
	else if(fnum == 5)
	{
		float xcen_new = xcen;
		float ycen_new = zcen;
		float zcen_new = -ycen;

		xcen = xcen_new;
		ycen = ycen_new;
		zcen = zcen_new;

		float dx_new = dx;
		float dy_new = dz;
		float dz_new = -dy;

		dx = dx_new;
		dy = dy_new;
		dz = dz_new;

	}
	else if(fnum == 6)
	{
		float xcen_new = xcen;
		float ycen_new = -zcen;
		float zcen_new = ycen;

		xcen = xcen_new;
		ycen = ycen_new;
		zcen = zcen_new;

		float dx_new = dx;
		float dy_new = -dz;
		float dz_new = dy;

		dx = dx_new;
		dy = dy_new;
		dz = dz_new;
	}

	

	/*********  detect nearest four images  *********/

	float t;
	float row_val;
	float col_val;

	
	t = (-1.0-zcen)/dz;
	col_val = (xcen+dx*t+3.0)/6.0*((float)lfcam_num_width-1.0);
	row_val = ((float)lfcam_num_height-1.0)-(ycen+dy*t+3.0)/6.0*((float)lfcam_num_height-1.0);

	
	float row_val_f = floor(row_val);
	float col_val_f = floor(col_val);

	float dis_col[2];
	dis_col[0] = col_val - col_val_f;
	dis_col[1] = 1.0 - dis_col[0];
	float dis_row[2];
	dis_row[0] = row_val - row_val_f;
	dis_row[1] = 1.0 - dis_row[0];


	/*********  combine pixel value  *********/

	int2 locate_src;

	float dis;

	float4 color_all;
	float4 color_temp;
	uchar depth_temp;


	color_all.s0 = 0;
	color_all.s1 = 0;
	color_all.s2 = 0;
	color_all.s3 = 1.0;

	float weight_all = 0;
	uchar depth_true = 0;
	uchar depth_ther_max,depth_ther_min;
	depth_ther_max = 0;
	depth_ther_min = 255;

	float depth_scal_v = t_depth.s0*depth_scal/cos_angle;
	
	float px_depth = xcen + dx*depth_scal_v;
	float py_depth = ycen + dy*depth_scal_v;
	float pz_depth = zcen + dz*depth_scal_v;
	

	float4 pt_depth = {px_depth,py_depth,pz_depth,0};

	float t_up;
	float t_right;

	int lf_pix_row;
	int lf_pix_col;

	int boundary_flag;
	int boundary_count = 0;

	
	for(int i = 0; i <= 1; i++)
	{
		for(int j = 0; j <= 1; j++)
		{
			
			boundary_flag = 0;
			float4 lfcam_lookat = {0, 0, -1.0, 0};
			float4 lfcam_pos = {1.0, 1.0, 0, 0};
			
			float sub_pos_col = ((float)(col_val_f+j))/((float)(lfcam_num_width-1.0))*6.0-3.0;
			float sub_pos_row = 3.0-((float)(row_val_f+i))/((float)(lfcam_num_height-1.0))*6.0;

			float4 sub_pos = {sub_pos_col,sub_pos_row,0,0};
			
			lfcam_pos = lfcam_pos * sub_pos + lfcam_lookat;

			float4 lfcam_ray_dir = pt_depth - lfcam_pos;

			float tt = dot(lfcam_lookat,lfcam_lookat)/dot(lfcam_lookat,lfcam_ray_dir);
			float4 lf_ray_dir = lfcam_ray_dir*tt-lfcam_lookat;
			
			t_right = lf_ray_dir.s0;
			t_up = lf_ray_dir.s1;
			
			lf_pix_col = (int)((t_right+lfcam_fov/2.0)/lfcam_fov*(lfcam_res-1));
			lf_pix_row = (int)((t_up+lfcam_fov/2.0)/lfcam_fov*(lfcam_res-1));
			
			
			if(boundary_flag == 0)
			{
				locate_src.y =  (fnum-1)*lfcam_res*lfcam_num_height + ((lfcam_num_height-1.0)-(row_val_f+i))*lfcam_res + lf_pix_row; // some problem
				locate_src.x = (col_val_f+j)*lfcam_res + lf_pix_col;

				color_temp = convert_float4(lf_src[locate_src.x+locate_src.y*lfcam_res*lfcam_num_width])/255.0;	
				dis = (1.0-dis_row[i]) * (1.0-dis_col[j]);
				depth_temp = lf_dep[locate_src.x+locate_src.y*lfcam_res*lfcam_num_width];

				if(depth_temp < depth_ther_max && depth_temp > depth_ther_min)
				{
					if(depth_temp < 250)
					{
						depth_ther_max = depth_temp + 5;
					}
					else
					{
						depth_ther_max = 255;
					}

					if(depth_temp > 5)
					{
						depth_ther_min = depth_temp - 5;
					}
					else
					{
						depth_ther_min = 0;
					}
					
					color_all = color_all + color_temp*dis;
					weight_all += dis;
				}
				else if(depth_temp > depth_ther_max)
				{
					if(depth_temp < 250)
					{
						depth_ther_max = depth_temp + 5;
					}
					else
					{
						depth_ther_max = 255;
					}

					if(depth_temp > 5)
					{
						depth_ther_min = depth_temp - 5;
					}
					else
					{
						depth_ther_min = 0;
					}
					color_all = color_temp*dis;
					weight_all = dis;

				}			

				//boundary_count++;
			}
		}
			
	}

	color_all = color_all/weight_all;


	/*********  assign color to final pixel  *********/

	if(color_coded_slabs == 1)
	{
		float slab_idx[4];
		slab_idx[0] = 0.0;
		slab_idx[1] = 0.0;
		slab_idx[2] = 0.0;
		slab_idx[3] = 0.0;	
			
		if(fnum <= 4)
		{
			slab_idx[fnum-1] = 0.6;
			color_all.s0 = slab_idx[0];
			color_all.s1 = slab_idx[1];
			color_all.s2 = slab_idx[2];
			color_all.s3 = slab_idx[3];
		}
		
		
	}

	float4 color_fn;

	if(show_depth == 1)
	{

		color_fn.s0 = t_depth.s0;
		color_fn.s1 = t_depth.s0;
		color_fn.s2 = t_depth.s0;
		color_fn.s3 = 1.0;
	}
	else
	{
		color_fn.s0 = color_all.s0 ;
		color_fn.s1 = color_all.s1 ;
		color_fn.s2 = color_all.s2 ;
		color_fn.s3 = color_all.s3 ;

	}
	
	write_imagef(nview_dst, locate_dst, color_fn); 

	return;

}