'''
Copyright (c) 2019, Ameer Haj Ali (UC Berkeley), and Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
'''
import ray
import ray.tune as tune
from ray.rllib.agents import ppo
from envs.neurovec import NeuroVectorizerEnv
from ray.tune.registry import register_env
from ray.tune.logger import TBXLogger

ray.init()
register_env("autovec", lambda config:NeuroVectorizerEnv(config))

tune.run("PPO",
        # restore = "~/zjw/Courses/Adv_Compiler/proj/neuro-vectorizer/ray_results/test/",
        checkpoint_freq  = 1,
        name = "neurovectorizer_train",
        stop = {"episodes_total": 100000},
        # resources_per_trial={"cpu": 1, "gpu": 1},
        config={
            "sample_batch_size": 25,
            "train_batch_size": 500,
            "sgd_minibatch_size": 20,
            "num_sgd_iter":20,
            "lr":5e-3,
            #"vf_loss_coeff":0.5,
            "env": "autovec",
            "horizon":  1,
            "num_gpus": 1,
            "model":{'fcnet_hiddens':[32,32]},
            "num_workers": 1,
            "env_config":{'dirpath':'./training_data','new_rundir':'./new_garbage'}
            },
        loggers=[TBXLogger],
        local_dir="~/zjw/Courses/Adv_Compiler/proj/neuro-vectorizer/ray_results/"
)
